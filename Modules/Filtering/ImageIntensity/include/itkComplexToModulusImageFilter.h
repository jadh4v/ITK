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
#ifndef itkComplexToModulusImageFilter_h
#define itkComplexToModulusImageFilter_h

#include "itkUnaryGeneratorImageFilter.h"
#include "itkMath.h"

namespace itk
{
/** \class ComplexToModulusImageFilter
 * \brief Computes pixel-wise the Modulus of a complex image.
 *
 * \ingroup IntensityImageFilters  MultiThreaded
 * \ingroup ITKImageIntensity
 */
namespace Functor
{
template <typename TInput, typename TOutput>
class ComplexToModulus
{
public:
  bool
  operator==(const ComplexToModulus &) const
  {
    return true;
  }

  ITK_UNEQUAL_OPERATOR_MEMBER_FUNCTION(ComplexToModulus);

  inline TOutput
  operator()(const TInput & A) const
  {
    return (TOutput)(std::sqrt(A.real() * A.real() + A.imag() * A.imag()));
  }
};
} // namespace Functor

template <typename TInputImage, typename TOutputImage>
class ITK_TEMPLATE_EXPORT ComplexToModulusImageFilter : public UnaryGeneratorImageFilter<TInputImage, TOutputImage>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(ComplexToModulusImageFilter);

  /** Standard class type aliases. */
  using Self = ComplexToModulusImageFilter;
  using Superclass = UnaryGeneratorImageFilter<TInputImage, TOutputImage>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using FunctorType = Functor::ComplexToModulus<typename TInputImage::PixelType, typename TOutputImage::PixelType>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(ComplexToModulusImageFilter);

  using InputPixelType = typename TInputImage::PixelType;
  using OutputPixelType = typename TOutputImage::PixelType;
  using InputPixelValueType = typename NumericTraits<InputPixelType>::ValueType;

  itkConceptMacro(InputMultiplyOperatorCheck, (Concept::MultiplyOperator<InputPixelValueType>));

protected:
  ComplexToModulusImageFilter()
  {
#if !defined(ITK_WRAPPING_PARSER)
    Superclass::SetFunctor(FunctorType());
#endif
  }
  ~ComplexToModulusImageFilter() override = default;
};
} // end namespace itk

#endif
