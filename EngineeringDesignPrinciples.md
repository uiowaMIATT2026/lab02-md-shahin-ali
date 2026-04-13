# Engineering Design Principles

**Author:** Md Shahin Ali (mali25)

## What I am trying to do
I need to write a program that takes two 2D images and registers them.
The two images are circles but they are different sizes and at different locations.
img1 is a 30mm diameter circle at (50,50)mm and img2 is a 60mm circle at (200,200)mm.
The program needs to work fast because it will run more than 1 million times.

## My approach
Since the two circles differ in both position and size, I need a transform that handles
both translation and scaling. I chose itk::Similarity2DTransform because it covers
translation, rotation, and isotropic scale in one transform with only 4 parameters.
A plain TranslationTransform would not work here because it cannot handle the size difference.

## Metric
I am using MeanSquaresImageToImageMetric. Both images are the same modality (grayscale),
so mean squared intensity difference is a natural and fast choice.

## Optimizer
I am using RegularStepGradientDescentOptimizer. It is simple and reliable for problems
with few degrees of freedom like this one.

## Why this will scale to 1 million runs
- Similarity2DTransform has only 4 DOF so optimizer converges quickly
- Mean squares metric is fast to compute
- No unnecessary file I/O in the core registration loop

## What I expect the result to be
The scale factor should come out around 0.5 (img2 is twice the size of img1) and
the translation should be around (-150, -150)mm to shift from center (200,200) to (50,50).
