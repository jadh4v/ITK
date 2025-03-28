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

#include <fstream>
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMetaImageIO.h"
#include "itkTestingMacros.h"
#include <algorithm> // For min.

int
itkMetaImageStreamingWriterIOTest(int argc, char * argv[])
{
  if (argc < 3)
  {
    std::cerr << "Usage: " << itkNameOfTestExecutableMacro(argv) << " input output" << std::endl;
    return EXIT_FAILURE;
  }

  // We remove the output file
  itksys::SystemTools::RemoveFile(argv[2]);

  using PixelType = unsigned char;
  using ImageType = itk::Image<PixelType, 3>;

  const itk::MetaImageIO::Pointer metaImageIO = itk::MetaImageIO::New();

  using ReaderType = itk::ImageFileReader<ImageType>;
  using WriterType = itk::ImageFileWriter<ImageType>;

  auto reader = ReaderType::New();
  reader->SetImageIO(metaImageIO);
  reader->SetFileName(argv[1]);
  reader->SetUseStreaming(true);
  metaImageIO->SetUseStreamedReading(true);

  itk::SizeValueType numberOfPieces = 10;

  // We decide how we want to read the image and we split accordingly
  // The image is read slice by slice
  reader->UpdateOutputInformation();
  ImageType::SizeType fullsize = reader->GetOutput()->GetLargestPossibleRegion().GetSize();

  ImageType::IndexType index{};

  ImageType::SizeType size;
  size[0] = fullsize[0];
  size[1] = fullsize[1];
  size[2] = 0;

  numberOfPieces = std::min(numberOfPieces, fullsize[2]);
  const unsigned int zsize = fullsize[2] / numberOfPieces;

  // Setup the writer
  auto writer = WriterType::New();
  writer->SetFileName(argv[2]);

  for (unsigned int i = 0; i < numberOfPieces; ++i)
  {
    std::cout << "Reading piece " << i + 1 << " of " << numberOfPieces << std::endl;

    index[2] += size[2];

    // At the end we need to adjust the size to make sure
    // we are reading everything
    if (i == numberOfPieces - 1)
    {
      size[2] = fullsize[2] - index[2];
    }
    else
    {
      size[2] = zsize;
    }

    const ImageType::RegionType region{ index, size };

    reader->GetOutput()->SetRequestedRegion(region);

    std::cout << "Requested region = " << std::endl;
    std::cout << region << std::endl;

    try
    {
      reader->Update();
    }
    catch (const itk::ExceptionObject & ex)
    {
      std::cout << "ERROR : " << ex << std::endl;
      return EXIT_FAILURE;
    }

    // Write the image
    itk::ImageIORegion ioregion(3);

    itk::ImageIORegion::IndexType index2;
    index2.push_back(index[0]);
    index2.push_back(index[1]);
    index2.push_back(index[2]);

    itk::ImageIORegion::SizeType size2;
    size2.push_back(size[0]);
    size2.push_back(size[1]);
    size2.push_back(size[2]);

    ioregion.SetIndex(index2);
    ioregion.SetSize(size2);

    writer->SetIORegion(ioregion);
    writer->SetInput(reader->GetOutput());

    try
    {
      writer->Update();
    }
    catch (const itk::ExceptionObject & err)
    {
      std::cerr << "ExceptionObject caught !" << std::endl;
      std::cerr << err << std::endl;
      return EXIT_FAILURE;
    }
  } // end for pieces


  return EXIT_SUCCESS;
}
