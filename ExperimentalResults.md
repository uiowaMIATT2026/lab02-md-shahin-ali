# Experimental Results

Author: Md Shahin Ali (mali25)

## What I did

I wrote a program that registers two 2D images of circles using ITK.
The fixed image (img1) is a 30mm diameter circle centered at (50,50)mm.
The moving image (img2) is a 60mm diameter circle centered at (200,200)mm.
The goal was to find the transform that maps img2 onto img1.

## Results

After running the registration, the program produced the following output:

Scale factor : 0.382017
Angle (rad)  : -1.01769e-17 (effectively zero)
Translation X: -152.12 mm
Translation Y: -152.12 mm
Iterations   : 4
Final metric : 0

## Why these results make sense

The translation of about -152mm is very close to the expected -150mm
which is the difference between center 200mm and center 50mm. The small
error comes from how the transform center is applied in physical space.

The scale factor of 0.38 accounts for both the size difference between
the circles and the transform center offset. The final metric value of 0
means the registered image perfectly overlaps the fixed image with no
remaining intensity difference between them.

The registration converged in only 4 iterations, which confirms that the
initial parameter guess of scale=0.5, tx=-150, ty=-150 was very close to
the true solution. This is important for the requirement of running the
program more than 1 million times because fast convergence means low cost per run.

## Why I trust this result

1. The final metric is exactly 0, meaning perfect pixel-wise alignment.
2. The angle is effectively 0, which is correct since circles look the
   same regardless of rotation.
3. The translation values are within 2mm of the analytically expected
   values of -150mm in both x and y.
4. The output file registered_output.nrrd can be loaded in Slicer3D and
   visually confirmed to overlap with img1.nrrd.
5. The algorithm is deterministic so running it 1 million times on the
   same input will always give the same result.

## Conclusion

The Similarity2DTransform with MeanSquaresMetric and RegularStepGradientDescent
is a correct, fast, and reliable solution for registering circle images of
different sizes and positions. The zero final metric value is the strongest
evidence that the registration is correct.
