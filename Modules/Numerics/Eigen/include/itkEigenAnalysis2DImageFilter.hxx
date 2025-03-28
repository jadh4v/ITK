/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkEigenAnalysis2DImageFilter_hxx
#define itkEigenAnalysis2DImageFilter_hxx

#include "itkImageRegionIteratorWithIndex.h"
#include "itkProgressReporter.h"

namespace itk
{

template <typename TInputImage, typename TEigenValueImage, typename TEigenVectorImage>
EigenAnalysis2DImageFilter<TInputImage, TEigenValueImage, TEigenVectorImage>::EigenAnalysis2DImageFilter()
{
  this->SetNumberOfRequiredInputs(3);
  ProcessObject::MakeRequiredOutputs(*this, 3);
  static_assert(EigenVectorType::Dimension == 2, "Error: PixelType of EigenVector Image must have exactly 2 elements!");
}

template <typename TInputImage, typename TEigenValueImage, typename TEigenVectorImage>
void
EigenAnalysis2DImageFilter<TInputImage, TEigenValueImage, TEigenVectorImage>::SetInput1(TInputImage * image)
{
  this->SetNthInput(0, image);
}

template <typename TInputImage, typename TEigenValueImage, typename TEigenVectorImage>
void
EigenAnalysis2DImageFilter<TInputImage, TEigenValueImage, TEigenVectorImage>::SetInput2(TInputImage * image)
{
  this->SetNthInput(1, image);
}

template <typename TInputImage, typename TEigenValueImage, typename TEigenVectorImage>
void
EigenAnalysis2DImageFilter<TInputImage, TEigenValueImage, TEigenVectorImage>::SetInput3(TInputImage * image)
{
  this->SetNthInput(2, image);
}

template <typename TInputImage, typename TEigenValueImage, typename TEigenVectorImage>
auto
EigenAnalysis2DImageFilter<TInputImage, TEigenValueImage, TEigenVectorImage>::GetMaxEigenValue()
  -> EigenValueImageType *
{
  return dynamic_cast<EigenValueImageType *>(this->ProcessObject::GetOutput(0));
}

template <typename TInputImage, typename TEigenValueImage, typename TEigenVectorImage>
auto
EigenAnalysis2DImageFilter<TInputImage, TEigenValueImage, TEigenVectorImage>::GetMinEigenValue()
  -> EigenValueImageType *
{
  return dynamic_cast<EigenValueImageType *>(this->ProcessObject::GetOutput(1));
}

template <typename TInputImage, typename TEigenValueImage, typename TEigenVectorImage>
auto
EigenAnalysis2DImageFilter<TInputImage, TEigenValueImage, TEigenVectorImage>::GetMaxEigenVector()
  -> EigenVectorImageType *
{
  auto * eigenVector = dynamic_cast<EigenVectorImageType *>(this->ProcessObject::GetOutput(2));

  if (eigenVector)
  {
    return eigenVector;
  }

  itkWarningMacro(

    << "EigenAnalysis2DImageFilter::GetMaxEigenVector(): dynamic_cast has failed. A reinterpret_cast is being "
       "attempted."
    << std::endl
    << "Type name is: " << typeid(*this->GetOutput(2)).name());
  return reinterpret_cast<EigenVectorImageType *>(this->ProcessObject::GetOutput(2));
}

template <typename TInputImage, typename TEigenValueImage, typename TEigenVectorImage>
DataObject::Pointer
EigenAnalysis2DImageFilter<TInputImage, TEigenValueImage, TEigenVectorImage>::MakeOutput(
  DataObjectPointerArraySizeType idx)
{
  DataObject::Pointer output;

  switch (idx)
  {
    case 0:
      output = (EigenValueImageType::New()).GetPointer();
      break;
    case 1:
      output = (EigenValueImageType::New()).GetPointer();
      break;
    case 2:
      output = (EigenVectorImageType::New()).GetPointer();
      break;
  }
  return output.GetPointer();
}

template <typename TInputImage, typename TEigenValueImage, typename TEigenVectorImage>
void
EigenAnalysis2DImageFilter<TInputImage, TEigenValueImage, TEigenVectorImage>::GenerateData()
{
  const typename TInputImage::ConstPointer inputPtr1(dynamic_cast<const TInputImage *>((ProcessObject::GetInput(0))));

  const typename TInputImage::ConstPointer inputPtr2(dynamic_cast<const TInputImage *>((ProcessObject::GetInput(1))));

  const typename TInputImage::ConstPointer inputPtr3(dynamic_cast<const TInputImage *>((ProcessObject::GetInput(2))));

  const EigenValueImagePointer  outputPtr1 = this->GetMaxEigenValue();
  const EigenValueImagePointer  outputPtr2 = this->GetMinEigenValue();
  const EigenVectorImagePointer outputPtr3 = this->GetMaxEigenVector();

  outputPtr1->SetBufferedRegion(inputPtr1->GetBufferedRegion());
  outputPtr2->SetBufferedRegion(inputPtr1->GetBufferedRegion());
  outputPtr3->SetBufferedRegion(inputPtr1->GetBufferedRegion());

  outputPtr1->Allocate();
  outputPtr2->Allocate();
  outputPtr3->Allocate();

  const EigenValueImageRegionType region = outputPtr1->GetRequestedRegion();

  ImageRegionConstIteratorWithIndex<TInputImage> inputIt1(inputPtr1, region);
  ImageRegionConstIteratorWithIndex<TInputImage> inputIt2(inputPtr2, region);
  ImageRegionConstIteratorWithIndex<TInputImage> inputIt3(inputPtr3, region);

  ImageRegionIteratorWithIndex<EigenValueImageType>  outputIt1(outputPtr1, region);
  ImageRegionIteratorWithIndex<EigenValueImageType>  outputIt2(outputPtr2, region);
  ImageRegionIteratorWithIndex<EigenVectorImageType> outputIt3(outputPtr3, region);

  constexpr EigenVectorType nullVector{};

  // support progress methods/callbacks
  ProgressReporter progress(this, 0, region.GetNumberOfPixels());

  inputIt1.GoToBegin();
  inputIt2.GoToBegin();
  inputIt3.GoToBegin();

  outputIt1.GoToBegin();
  outputIt2.GoToBegin();
  outputIt3.GoToBegin();

  EigenVectorType eigenVector;

  while (!inputIt1.IsAtEnd())
  {
    const auto xx = static_cast<double>(inputIt1.Get());
    const auto xy = static_cast<double>(inputIt2.Get());
    const auto yy = static_cast<double>(inputIt3.Get());

    const double dxy = xx - yy;
    const double sxy = xx + yy;

    const double S = std::sqrt(dxy * dxy + 4.0 * xy * xy);

    const double pp = (sxy + S) / 2.0;
    const double qq = (sxy - S) / 2.0;

    outputIt1.Set(pp);
    outputIt2.Set(qq);

    eigenVector[0] = static_cast<VectorComponentType>((-dxy - S) / 2.0);
    eigenVector[1] = static_cast<VectorComponentType>(-xy);

    const VectorComponentType norm = eigenVector.GetNorm();
    if (norm > 1e-30)
    {
      outputIt3.Set(eigenVector / norm);
    }
    else
    {
      outputIt3.Set(nullVector);
    }

    ++inputIt1;
    ++inputIt2;
    ++inputIt3;

    ++outputIt1;
    ++outputIt2;
    ++outputIt3;

    progress.CompletedPixel();
  }
}
} // end namespace itk

#endif
