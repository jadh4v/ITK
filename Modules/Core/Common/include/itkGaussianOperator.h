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
#ifndef itkGaussianOperator_h
#define itkGaussianOperator_h

#include "itkNeighborhoodOperator.h"
#include <cmath>
namespace itk
{
/**
 * \class GaussianOperator
 * \brief A NeighborhoodOperator whose coefficients are a one
 * dimensional, discrete Gaussian kernel.
 *
 * GaussianOperator can be used to perform Gaussian blurring
 * by taking its inner product with a Neighborhood
 * (NeighborhoodIterator) that is swept across an image region.
 * It is a directional operator.  N successive applications
 * oriented along each dimensional direction will effect separable,
 * efficient, N-D Gaussian blurring of an image region.
 *
 * GaussianOperator takes two parameters:
 *
 * (1) The floating-point variance of the desired Gaussian function.
 *
 * (2) The "maximum error" allowed in the discrete Gaussian
 * function.  "Maximum error" is defined as the difference between the area
 * under the discrete Gaussian curve and the area under the continuous
 * Gaussian. Maximum error affects the Gaussian operator size. Care should
 * be taken not to make this value too small relative to the variance
 * lest the operator size become unreasonably large.
 *
 * References:
 * The Gaussian kernel contained in this operator was described
 * in \cite lindeberg1991.
 *
 * \note GaussianOperator does not have any user-declared "special member function",
 * following the C++ Rule of Zero: the compiler will generate them if necessary.
 *
 * \sa NeighborhoodOperator
 * \sa NeighborhoodIterator
 * \sa Neighborhood
 *
 * \ingroup Operators
 * \ingroup ITKCommon
 *
 * \sphinx
 * \sphinxexample{Core/Common/CreateGaussianKernel,Create Gaussian Kernel}
 * \endsphinx
 */
template <typename TPixel, unsigned int VDimension = 2, typename TAllocator = NeighborhoodAllocator<TPixel>>
class ITK_TEMPLATE_EXPORT GaussianOperator : public NeighborhoodOperator<TPixel, VDimension, TAllocator>
{
public:
  /** Standard class type aliases. */
  using Self = GaussianOperator;
  using Superclass = NeighborhoodOperator<TPixel, VDimension, TAllocator>;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(GaussianOperator);

  /** Sets the desired variance of the Gaussian kernel. */
  void
  SetVariance(const double variance)
  {
    m_Variance = variance;
  }

  /** Sets the desired maximum error of the gaussian approximation.  Maximum
   * error is the difference between the area under the discrete Gaussian curve
   * and the area under the continuous Gaussian. Maximum error affects the
   * Gaussian operator size. The value must be between 0.0 and 1.0. */
  void
  SetMaximumError(const double max_error)
  {
    if (max_error >= 1 || max_error <= 0)
    {
      itkExceptionMacro("Maximum Error Must be in the range [ 0.0 , 1.0 ]");
    }

    m_MaximumError = max_error;
  }

  /** Returns the variance of the Gaussian (scale) for the operator. */
  double
  GetVariance()
  {
    return m_Variance;
  }

  /** Returns the maximum error of the gaussian approximation.  Maximum error is
   * the difference between the area under the discrete Gaussian curve and the
   * area under the continuous Gaussian. Maximum error affects the Gaussian
   * operator size. */
  double
  GetMaximumError()
  {
    return m_MaximumError;
  }

  /** Sets a limit for growth of the kernel.  Small maximum error values with
   *  large variances will yield very large kernel sizes.  This value can be
   *  used to truncate a kernel in such instances.  A warning will be given on
   *  truncation of the kernel. */
  void
  SetMaximumKernelWidth(unsigned int n)
  {
    m_MaximumKernelWidth = n;
  }

  /** Returns the maximum allowed kernel width. */
  unsigned int
  GetMaximumKernelWidth() const
  {
    return m_MaximumKernelWidth;
  }

  void
  PrintSelf(std::ostream & os, Indent indent) const override
  {
    Superclass::PrintSelf(os, indent);

    os << indent << "Variance: " << m_Variance << std::endl;
    os << indent << "MaximumError: " << m_MaximumError << std::endl;
    os << indent << "MaximumKernelWidth: " << m_MaximumKernelWidth << std::endl;
  }

  /** Get the value of the debug flag.
   *  Mimics the itk::Object interface so that itkDebugMacro
   *  can be used in selective printouts from Gaussian kernel generation.*/
  bool
  GetDebug() const
  {
    return m_Debug;
  }
  /** Turn debugging output on.  */
  void
  DebugOn() const
  {
    m_Debug = true;
  }
  /** Turn debugging output off.  */
  void
  DebugOff() const
  {
    m_Debug = false;
  }
  /** Set the value of the debug flag. A non-zero value turns debugging on. */
  void
  SetDebug(bool debugFlag) const
  {
    m_Debug = debugFlag;
  }

public:
  /** Returns the value of the modified Bessel function I0(x) at a point x >= 0.
   */
  double
  ModifiedBesselI0(double);

  /** Returns the value of the modified Bessel function I1(x) at a point x,
   * x real.  */
  double
  ModifiedBesselI1(double);

  /** Returns the value of the modified Bessel function Ik(x) at a point x>=0,
   * where k>=2. */
  double
  ModifiedBesselI(int, double);

protected:
  /** Type alias support for coefficient vector type.*/
  using typename Superclass::CoefficientVector;

  /** Calculates operator coefficients. */
  CoefficientVector
  GenerateCoefficients() override;

  /** Arranges coefficients spatially in the memory buffer. */
  void
  Fill(const CoefficientVector & coeff) override
  {
    this->FillCenteredDirectional(coeff);
  }

private:
  /** Desired variance of the discrete Gaussian function. */
  double m_Variance{ 1 };

  /** Difference between the areas under the curves of the continuous and
   * discrete Gaussian functions. */
  double m_MaximumError{ .01 };

  /** Maximum kernel size allowed.  This value is used to truncate a kernel
   *  that has grown too large.  A warning is given when the specified maximum
   *  error causes the kernel to exceed this size. */
  unsigned int m_MaximumKernelWidth{ 30 };

  /** Enable/disable kernel generation debug warnings */
  mutable bool m_Debug{ false };
};
} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkGaussianOperator.hxx"
#endif

#endif
