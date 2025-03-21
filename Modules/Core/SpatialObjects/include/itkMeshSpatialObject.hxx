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
#ifndef itkMeshSpatialObject_hxx
#define itkMeshSpatialObject_hxx

#include "itkSize.h"

namespace itk
{

template <typename TMesh>
MeshSpatialObject<TMesh>::MeshSpatialObject()
{
  this->SetTypeName("MeshSpatialObject");

  this->Clear();

  this->Update();
}

template <typename TMesh>
void
MeshSpatialObject<TMesh>::Clear()
{
  Superclass::Clear();

  m_Mesh = MeshType::New();
#if !defined(ITK_LEGACY_REMOVE)
  m_PixelType = typeid(typename TMesh::PixelType).name();
#endif
  m_IsInsidePrecisionInObjectSpace = 1;

  this->Modified();
}

template <typename TMesh>
bool
MeshSpatialObject<TMesh>::IsInsideInObjectSpace(const PointType & point) const
{
  if (this->GetMyBoundingBoxInObjectSpace()->IsInside(point))
  {
    const typename MeshType::CellsContainerPointer   cells = m_Mesh->GetCells();
    typename MeshType::CellsContainer::ConstIterator it = cells->Begin();
    while (it != cells->End())
    {
      using CoordinateType = typename MeshType::CoordinateType;
      CoordinateType position[Dimension];
      for (unsigned int i = 0; i < Dimension; ++i)
      {
        position[i] = point[i];
      }

      // If this is a triangle cell we need to check the distance
      if (it.Value()->GetNumberOfPoints() == 3)
      {
        double     minDist = 0.0;
        const bool pointIsInsideInObjectSpace =
          it.Value()->EvaluatePosition(position, m_Mesh->GetPoints(), nullptr, nullptr, &minDist, nullptr);

        if (pointIsInsideInObjectSpace && minDist <= this->m_IsInsidePrecisionInObjectSpace)
        {
          return true;
        }
      }
      else
      {
        if (it.Value()->EvaluatePosition(position, m_Mesh->GetPoints(), nullptr, nullptr, nullptr, nullptr))
        {
          return true;
        }
      }
      ++it;
    }
  }

  return false;
}

template <typename TMesh>
void
MeshSpatialObject<TMesh>::ComputeMyBoundingBox()
{
  PointType pnt1;
  PointType pnt2;
  for (unsigned int i = 0; i < this->ObjectDimension; ++i)
  {
    pnt1[i] = m_Mesh->GetBoundingBox()->GetBounds()[2 * i];
    pnt2[i] = m_Mesh->GetBoundingBox()->GetBounds()[2 * i + 1];
  }

  this->GetModifiableMyBoundingBoxInObjectSpace()->SetMinimum(m_Mesh->GetBoundingBox()->GetMinimum());
  this->GetModifiableMyBoundingBoxInObjectSpace()->SetMaximum(m_Mesh->GetBoundingBox()->GetMaximum());
  this->GetModifiableMyBoundingBoxInObjectSpace()->ComputeBoundingBox();
}

template <typename TMesh>
void
MeshSpatialObject<TMesh>::SetMesh(MeshType * mesh)
{
  if (m_Mesh != mesh)
  {
    m_Mesh = mesh;
    this->Modified();
  }
}

template <typename TMesh>
auto
MeshSpatialObject<TMesh>::GetMesh() -> MeshType *
{
  return m_Mesh.GetPointer();
}

template <typename TMesh>
auto
MeshSpatialObject<TMesh>::GetMesh() const -> const MeshType *
{
  return m_Mesh.GetPointer();
}

template <typename TMesh>
typename LightObject::Pointer
MeshSpatialObject<TMesh>::InternalClone() const
{
  // Default implementation just copies the parameters from
  // this to new transform.
  typename LightObject::Pointer loPtr = Superclass::InternalClone();

  const typename Self::Pointer rval = dynamic_cast<Self *>(loPtr.GetPointer());
  if (rval.IsNull())
  {
    itkExceptionMacro("downcast to type " << this->GetNameOfClass() << " failed.");
  }
  rval->SetMesh(this->GetMesh()->Clone());
  rval->SetIsInsidePrecisionInObjectSpace(this->GetIsInsidePrecisionInObjectSpace());

  return loPtr;
}

template <typename TMesh>
void
MeshSpatialObject<TMesh>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  itkPrintSelfObjectMacro(Mesh);

#if !defined(ITK_LEGACY_REMOVE)
  os << indent << "PixelType: " << m_PixelType << std::endl;
#endif
  os << indent << "IsInsidePrecisionInObjectSpace: " << m_IsInsidePrecisionInObjectSpace << std::endl;
}

template <typename TMesh>
ModifiedTimeType
MeshSpatialObject<TMesh>::GetMTime() const
{
  ModifiedTimeType       latestMTime = Superclass::GetMTime();
  const ModifiedTimeType MeshMTime = m_Mesh->GetMTime();

  if (MeshMTime > latestMTime)
  {
    latestMTime = MeshMTime;
  }

  return latestMTime;
}
} // end namespace itk

#endif //__MeshSpatialObject_hxx
