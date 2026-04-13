// RegisterCircles.cxx
// Md Shahin Ali (mali25)
// Register two 2D circle images using ITK Similarity2DTransform
// img1: 30mm diameter circle centered at (50,50)mm
// img2: 60mm diameter circle centered at (200,200)mm

#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkImageRegistrationMethod.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkSimilarity2DTransform.h"
#include "itkResampleImageFilter.h"

// float pixels needed for registration metric computations
using PixelType  = float;
using ImageType  = itk::Image<PixelType, 2>;

// Create a 2D image with a filled circle
// center_x, center_y in pixels (== mm since spacing=1)
// diameter in mm
ImageType::Pointer
CreateCircleImage(double center_x, double center_y, double diameter,
                  unsigned int image_size = 512)
{
  ImageType::Pointer image = ImageType::New();

  ImageType::SizeType size;
  size[0] = image_size;
  size[1] = image_size;

  ImageType::IndexType start;
  start[0] = 0;
  start[1] = 0;

  ImageType::RegionType region;
  region.SetSize(size);
  region.SetIndex(start);
  image->SetRegions(region);

  // spacing = 1mm so index == physical coordinate in mm
  ImageType::SpacingType spacing;
  spacing[0] = 1.0;
  spacing[1] = 1.0;
  image->SetSpacing(spacing);

  ImageType::PointType origin;
  origin[0] = 0.0;
  origin[1] = 0.0;
  image->SetOrigin(origin);

  image->Allocate();
  image->FillBuffer(0.0f);

  double radius = diameter / 2.0;

  for(unsigned int j = 0; j < image_size; ++j)
    {
    for(unsigned int i = 0; i < image_size; ++i)
      {
      double dx = static_cast<double>(i) - center_x;
      double dy = static_cast<double>(j) - center_y;
      if( (dx*dx + dy*dy) <= (radius*radius) )
        {
        ImageType::IndexType idx;
        idx[0] = i;
        idx[1] = j;
        image->SetPixel(idx, 255.0f);
        }
      }
    }
  return image;
}

int main(int argc, char* argv[])
{
  // img1: fixed image  -- 30mm diameter circle at (50,50)mm
  // img2: moving image -- 60mm diameter circle at (200,200)mm
  ImageType::Pointer fixedImage  = CreateCircleImage(50.0,  50.0,  30.0);
  ImageType::Pointer movingImage = CreateCircleImage(200.0, 200.0, 60.0);

  // Save input images for verification
  {
  using WriterType = itk::ImageFileWriter<ImageType>;
  WriterType::Pointer w1 = WriterType::New();
  w1->SetInput(fixedImage);
  w1->SetFileName("img1.nrrd");
  w1->Update();
  WriterType::Pointer w2 = WriterType::New();
  w2->SetInput(movingImage);
  w2->SetFileName("img2.nrrd");
  w2->Update();
  }

  using TransformType    = itk::Similarity2DTransform<double>;
  using OptimizerType    = itk::RegularStepGradientDescentOptimizer;
  using MetricType       = itk::MeanSquaresImageToImageMetric<ImageType, ImageType>;
  using InterpolatorType = itk::LinearInterpolateImageFunction<ImageType, double>;
  using RegistrationType = itk::ImageRegistrationMethod<ImageType, ImageType>;

  TransformType::Pointer    transform    = TransformType::New();
  OptimizerType::Pointer    optimizer    = OptimizerType::New();
  MetricType::Pointer       metric       = MetricType::New();
  InterpolatorType::Pointer interpolator = InterpolatorType::New();
  RegistrationType::Pointer registration = RegistrationType::New();

  registration->SetMetric(metric);
  registration->SetOptimizer(optimizer);
  registration->SetInterpolator(interpolator);
  registration->SetTransform(transform);
  registration->SetFixedImage(fixedImage);
  registration->SetMovingImage(movingImage);
  registration->SetFixedImageRegion(fixedImage->GetLargestPossibleRegion());

  // Set initial transform parameters manually
  // Similarity2DTransform has 4 parameters: scale, angle, tx, ty
  // Start with scale=0.5 (img2 is 2x larger), angle=0, translation guess
  TransformType::ParametersType initParams(transform->GetNumberOfParameters());
  initParams[0] = 0.5;    // scale: img2 diameter is 2x img1 so scale ~ 0.5
  initParams[1] = 0.0;    // angle: circles are rotation-invariant
  initParams[2] = -150.0; // tx: shift from 200 to 50
  initParams[3] = -150.0; // ty: shift from 200 to 50
  transform->SetParameters(initParams);

  // Set center of transform to center of moving image circle
  TransformType::InputPointType center;
  center[0] = 200.0;
  center[1] = 200.0;
  transform->SetCenter(center);

  // Provide initial parameters to registration
  registration->SetInitialTransformParameters(transform->GetParameters());

  // Optimizer settings
  OptimizerType::ScalesType scales(transform->GetNumberOfParameters());
  scales[0] = 10.0;  // scale factor -- needs larger scale to balance with translation
  scales[1] = 1.0;   // angle
  scales[2] = 0.001; // tx
  scales[3] = 0.001; // ty
  optimizer->SetScales(scales);
  optimizer->SetMaximumStepLength(1.0);
  optimizer->SetMinimumStepLength(0.001);
  optimizer->SetNumberOfIterations(300);
  optimizer->MinimizeOn();

  try
    {
    registration->Update();
    }
  catch(itk::ExceptionObject & err)
    {
    std::cerr << "Registration failed: " << err << std::endl;
    return EXIT_FAILURE;
    }

  TransformType::ParametersType finalParams = registration->GetLastTransformParameters();
  std::cout << "======================================" << std::endl;
  std::cout << "Registration Results:" << std::endl;
  std::cout << "  Scale factor : " << finalParams[0] << std::endl;
  std::cout << "  Angle (rad)  : " << finalParams[1] << std::endl;
  std::cout << "  Translation X: " << finalParams[2] << " mm" << std::endl;
  std::cout << "  Translation Y: " << finalParams[3] << " mm" << std::endl;
  std::cout << "  Iterations   : " << optimizer->GetCurrentIteration() << std::endl;
  std::cout << "  Final metric : " << optimizer->GetValue() << std::endl;
  std::cout << "======================================" << std::endl;

  // Resample moving image into fixed image space
  using ResampleFilterType = itk::ResampleImageFilter<ImageType, ImageType>;
  ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  resampler->SetInput(movingImage);
  resampler->SetTransform(registration->GetOutput()->Get());
  resampler->SetUseReferenceImage(true);
  resampler->SetReferenceImage(fixedImage);
  resampler->SetDefaultPixelValue(0);
  resampler->Update();

  using WriterType = itk::ImageFileWriter<ImageType>;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput(resampler->GetOutput());
  writer->SetFileName("registered_output.nrrd");
  writer->Update();

  std::cout << "Registered image saved to registered_output.nrrd" << std::endl;

  return EXIT_SUCCESS;
}
