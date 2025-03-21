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

#include "itkFastMarchingQuadEdgeMeshFilterBase.h"
#include "itkQuadEdgeMeshExtendedTraits.h"
#include "itkRegularSphereMeshSource.h"
#include "itkFastMarchingThresholdStoppingCriterion.h"
#include "itkMeshFileWriter.h"
#include "itkTestingMacros.h"

int
itkFastMarchingQuadEdgeMeshFilterBaseTest(int, char *[])
{
  using PixelType = float;
  using CoordType = double;

  constexpr unsigned int Dimension = 3;

  using Traits = itk::QuadEdgeMeshExtendedTraits<PixelType, // type of data for vertices
                                                 Dimension, // geometrical dimension of space
                                                 2,         // Mac topological dimension of a cell
                                                 CoordType, // type for point coordinate
                                                 CoordType, // type for interpolation weight
                                                 PixelType, // type of data for cell
                                                 bool,      // type of data for primal edges
                                                 bool       // type of data for dual edges
                                                 >;

  using MeshType = itk::QuadEdgeMesh<PixelType, Dimension, Traits>;

  using FastMarchingType = itk::FastMarchingQuadEdgeMeshFilterBase<MeshType, MeshType>;

  const MeshType::PointType center{};

  using SphereSourceType = itk::RegularSphereMeshSource<MeshType>;
  auto sphere_filter = SphereSourceType::New();
  sphere_filter->SetCenter(center);
  sphere_filter->SetResolution(5);
  sphere_filter->Update();

  const MeshType::Pointer sphere_output = sphere_filter->GetOutput();

  const MeshType::PointsContainerConstPointer points = sphere_output->GetPoints();

  MeshType::PointsContainerConstIterator       p_it = points->Begin();
  const MeshType::PointsContainerConstIterator p_end = points->End();

  while (p_it != p_end)
  {
    sphere_output->SetPointData(p_it->Index(), 1.);
    ++p_it;
  }

  using NodePairType = FastMarchingType::NodePairType;
  //  using NodeContainerType = FastMarchingType::NodeContainerType;
  using NodePairContainerType = FastMarchingType::NodePairContainerType;

  auto trial = NodePairContainerType::New();

  const NodePairType node_pair(0, 0.);
  trial->push_back(node_pair);

  using CriterionType = itk::FastMarchingThresholdStoppingCriterion<MeshType, MeshType>;
  auto criterion = CriterionType::New();
  criterion->SetThreshold(100.);

  auto fmm_filter = FastMarchingType::New();
  fmm_filter->SetInput(sphere_output);
  fmm_filter->SetTrialPoints(trial);
  fmm_filter->SetStoppingCriterion(criterion);

  ITK_TRY_EXPECT_NO_EXCEPTION(fmm_filter->Update());


  using WriterType = itk::MeshFileWriter<MeshType>;
  auto writer = WriterType::New();
  writer->SetInput(fmm_filter->GetOutput());
  writer->SetFileName("itkFastMarchingQuadEdgeMeshFilterBase.vtk");
  writer->Update();


  return EXIT_SUCCESS;
}
