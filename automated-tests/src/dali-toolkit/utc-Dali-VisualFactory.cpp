/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <stdlib.h>
#include <dali-toolkit-test-suite-utils.h>
#include <toolkit-event-thread-callback.h>
#include <dali/public-api/rendering/renderer.h>
#include <dali/public-api/rendering/texture-set.h>
#include <dali/public-api/rendering/shader.h>
#include <dali-toolkit/devel-api/visual-factory/visual-factory.h>

using namespace Dali;
using namespace Dali::Toolkit;

namespace
{
typedef NinePatchImage::StretchRanges StretchRanges;

const char* TEST_IMAGE_FILE_NAME =  "gallery_image_01.jpg";
const char* TEST_NPATCH_FILE_NAME =  "gallery_image_01.9.png";

const char* TEST_SVG_FILE_NAME = TEST_RESOURCE_DIR "/svg1.svg";
const char* TEST_OBJ_FILE_NAME = TEST_RESOURCE_DIR "/Cube.obj";
const char* TEST_MTL_FILE_NAME = TEST_RESOURCE_DIR "/ToyRobot-Metal.mtl";
const char* TEST_SIMPLE_OBJ_FILE_NAME = TEST_RESOURCE_DIR "/Cube-Points-Only.obj";
const char* TEST_SIMPLE_MTL_FILE_NAME = TEST_RESOURCE_DIR "/ToyRobot-Metal-Simple.mtl";

Integration::Bitmap* CreateBitmap( unsigned int imageWidth, unsigned int imageHeight, unsigned int initialColor, Pixel::Format pixelFormat )
{
  Integration::Bitmap* bitmap = Integration::Bitmap::New( Integration::Bitmap::BITMAP_2D_PACKED_PIXELS, ResourcePolicy::OWNED_RETAIN );
  Integration::PixelBuffer* pixbuffer = bitmap->GetPackedPixelsProfile()->ReserveBuffer( pixelFormat, imageWidth, imageHeight, imageWidth, imageHeight );
  unsigned int bytesPerPixel = GetBytesPerPixel( pixelFormat );

  memset( pixbuffer, initialColor, imageHeight * imageWidth * bytesPerPixel );

  return bitmap;
}

void InitialiseRegionsToZeroAlpha( Integration::Bitmap* image, unsigned int imageWidth, unsigned int imageHeight, Pixel::Format pixelFormat )
{
  PixelBuffer* pixbuffer = image->GetBuffer();
  unsigned int bytesPerPixel = GetBytesPerPixel( pixelFormat );

  for( unsigned int row = 0; row < imageWidth; ++row )
  {
    unsigned int pixelOffset = row * bytesPerPixel;
    pixbuffer[ pixelOffset + 3 ] = 0x00;
    pixelOffset += ( imageHeight - 1 ) * imageWidth * bytesPerPixel;
    pixbuffer[ pixelOffset + 3 ] = 0x00;
  }

  for ( unsigned int column = 0; column < imageHeight; ++column )
  {
    unsigned int pixelOffset = column * imageWidth * bytesPerPixel;
    pixbuffer[ pixelOffset + 3 ] = 0x00;
    pixelOffset += ( imageWidth -1 ) * bytesPerPixel;
    pixbuffer[ pixelOffset + 3 ] = 0x00;
  }
}

void AddStretchRegionsToImage( Integration::Bitmap* image, unsigned int imageWidth, unsigned int imageHeight, const StretchRanges& stretchRangesX, const StretchRanges& stretchRangesY, Pixel::Format pixelFormat )
{
  PixelBuffer* pixbuffer = image->GetBuffer();
  unsigned int bytesPerPixel = GetBytesPerPixel( pixelFormat );

  for(StretchRanges::ConstIterator it = stretchRangesX.Begin(); it != stretchRangesX.End(); ++it)
  {
    const Uint16Pair& range = *it;
    //since the stretch range is in the cropped image space, we need to offset by 1 to get it to the uncropped image space
    for( unsigned int column = range.GetX() + 1u; column < range.GetY() + 1u; ++column )
    {
      unsigned int pixelOffset = column * bytesPerPixel;
      pixbuffer[ pixelOffset ] = 0x00;
      pixbuffer[ pixelOffset + 1 ] = 0x00;
      pixbuffer[ pixelOffset + 2 ] = 0x00;
      pixbuffer[ pixelOffset + 3 ] = 0xFF;
    }
  }


  for(StretchRanges::ConstIterator it = stretchRangesY.Begin(); it != stretchRangesY.End(); ++it)
  {
    const Uint16Pair& range = *it;
    //since the stretch range is in the cropped image space, we need to offset by 1 to get it to the uncropped image space
    for( unsigned int row = range.GetX() + 1u; row < range.GetY() + 1u; ++row )
    {
      unsigned int pixelOffset = row * imageWidth * bytesPerPixel;
      pixbuffer[ pixelOffset ] = 0x00;
      pixbuffer[ pixelOffset + 1 ] = 0x00;
      pixbuffer[ pixelOffset + 2 ] = 0x00;
      pixbuffer[ pixelOffset + 3 ] = 0xFF;
    }
  }
}

void AddChildRegionsToImage( Integration::Bitmap* image, unsigned int imageWidth, unsigned int imageHeight, const Vector4& requiredChildRegion, Pixel::Format pixelFormat )
{
  PixelBuffer* pixbuffer = image->GetBuffer();
  unsigned int bytesPerPixel = GetBytesPerPixel( pixelFormat );

  Integration::Bitmap::PackedPixelsProfile* srcProfile = image->GetPackedPixelsProfile();
  unsigned int bufferStride = srcProfile->GetBufferStride();

  // Add bottom child region
  for( unsigned int column = requiredChildRegion.x; column < imageWidth - requiredChildRegion.z; ++column )
  {
    unsigned int pixelOffset = column * bytesPerPixel;
    pixelOffset += ( imageHeight - 1 ) * bufferStride;
    pixbuffer[ pixelOffset ] = 0x00;
    pixbuffer[ pixelOffset + 1 ] = 0x00;
    pixbuffer[ pixelOffset + 2 ] = 0x00;
    pixbuffer[ pixelOffset + 3 ] = 0xFF;
  }

  // Add right child region
  for ( unsigned int row = requiredChildRegion.y; row < imageHeight - requiredChildRegion.w; ++row )
  {
    unsigned int pixelOffset = row * bufferStride + ( imageWidth - 1 ) * bytesPerPixel;
    pixbuffer[ pixelOffset ] = 0x00;
    pixbuffer[ pixelOffset + 1 ] = 0x00;
    pixbuffer[ pixelOffset + 2 ] = 0x00;
    pixbuffer[ pixelOffset + 3 ] = 0xFF;
  }
}

Integration::ResourcePointer CustomizeNinePatch( TestApplication& application,
                                                 unsigned int ninePatchImageWidth,
                                                 unsigned int ninePatchImageHeight,
                                                 const StretchRanges& stretchRangesX,
                                                 const StretchRanges& stretchRangesY,
                                                 bool addChildRegion = false,
                                                 Vector4 requiredChildRegion = Vector4::ZERO )
{
  TestPlatformAbstraction& platform = application.GetPlatform();

  Pixel::Format pixelFormat = Pixel::RGBA8888;

  tet_infoline("Create Bitmap");
  platform.SetClosestImageSize(Vector2( ninePatchImageWidth, ninePatchImageHeight));
  Integration::Bitmap* bitmap = CreateBitmap( ninePatchImageWidth, ninePatchImageHeight, 0xFF, pixelFormat );

  tet_infoline("Clear border regions");
  InitialiseRegionsToZeroAlpha( bitmap, ninePatchImageWidth, ninePatchImageHeight, pixelFormat );

  tet_infoline("Add Stretch regions to Bitmap");
  AddStretchRegionsToImage( bitmap, ninePatchImageWidth, ninePatchImageHeight, stretchRangesX, stretchRangesY, pixelFormat );

  if( addChildRegion )
  {
    tet_infoline("Add Child regions to Bitmap");
    AddChildRegionsToImage( bitmap, ninePatchImageWidth, ninePatchImageHeight, requiredChildRegion, pixelFormat );
  }

  tet_infoline("Getting resource");
  Integration::ResourcePointer resourcePtr(bitmap);
  //platform.SetResourceLoaded( 0, Dali::Integration::ResourceBitmap, resourcePtr );
  platform.SetSynchronouslyLoadedResource( resourcePtr);

  return resourcePtr;
}

void TestVisualRender( ToolkitTestApplication& application,
                                Actor& actor,
                                Visual& visual,
                                std::size_t expectedSamplers = 0,
                                ImageDimensions imageDimensions = ImageDimensions(),
                                Integration::ResourcePointer resourcePtr = Integration::ResourcePointer())
{
  if( resourcePtr )
  {
    // set the image size, for test case, this needs to be set before loading started
    application.GetPlatform().SetClosestImageSize(  Vector2(imageDimensions.GetWidth(), imageDimensions.GetHeight()) );
  }

  actor.SetSize( 200.f, 200.f );
  Stage::GetCurrent().Add( actor );
  visual.SetSize( Vector2(200.f, 200.f) );
  visual.SetOnStage( actor );

  DALI_TEST_CHECK( actor.GetRendererCount() == 1u );

  application.SendNotification();
  application.Render();

  if( resourcePtr )
  {
    Integration::ResourceRequest* request = application.GetPlatform().GetRequest();
    if(request)
    {
      application.GetPlatform().SetResourceLoaded(request->GetId(), request->GetType()->id, resourcePtr );
    }
  }

  application.Render();
  application.SendNotification();

  if( resourcePtr )
  {
    DALI_TEST_CHECK( application.GetPlatform().WasCalled(TestPlatformAbstraction::LoadResourceFunc) ||
                     application.GetPlatform().WasCalled(TestPlatformAbstraction::LoadResourceSynchronouslyFunc ));
  }

  DALI_TEST_CHECK( actor.GetRendererCount() == 1u );

}

} // namespace


void dali_visual_factory_startup(void)
{
  test_return_value = TET_UNDEF;
}

void dali_visual_factory_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliVisualFactoryGet(void)
{
  ToolkitTestApplication application;
  tet_infoline( "UtcDaliVisualFactory" );

  //Register type
  TypeInfo type;
  type = TypeRegistry::Get().GetTypeInfo( "VisualFactory" );
  DALI_TEST_CHECK( type );
  BaseHandle handle = type.CreateInstance();
  DALI_TEST_CHECK( handle );

  VisualFactory factory;
  factory = VisualFactory::Get();
  DALI_TEST_CHECK( factory );

  VisualFactory newFactory = VisualFactory::Get();
  DALI_TEST_CHECK( newFactory );

  // Check that renderer factory is a singleton
  DALI_TEST_CHECK(factory == newFactory);

  END_TEST;
}

int UtcDaliVisualFactoryCopyAndAssignment(void)
{
  ToolkitTestApplication application;
  tet_infoline( "UtcDaliVisualFactoryCopyAndAssignment" );
  VisualFactory factory = VisualFactory::Get();

  VisualFactory factoryCopy( factory );
  DALI_TEST_CHECK(factory == factoryCopy);

  VisualFactory emptyFactory;
  VisualFactory emptyFactoryCopy( emptyFactory );
  DALI_TEST_CHECK(emptyFactory == emptyFactoryCopy);

  VisualFactory factoryEquals;
  factoryEquals = factory;
  DALI_TEST_CHECK(factory == factoryEquals);

  VisualFactory emptyFactoryEquals;
  emptyFactoryEquals = emptyFactory;
  DALI_TEST_CHECK( emptyFactory == emptyFactoryEquals );

  //self assignment
  factory = factory;
  DALI_TEST_CHECK( factory = factoryCopy );

  END_TEST;
}

int UtcDaliVisualFactoryGetColorVisual1(void)
{
  ToolkitTestApplication application;
  tet_infoline( "UtcDaliVisualFactoryGetColorVisual1:  Request color visual with a Property::Map" );

  VisualFactory factory = VisualFactory::Get();
  DALI_TEST_CHECK( factory );

  Property::Map propertyMap;
  Vector4 testColor( 1.f, 0.5f, 0.3f, 0.2f );
  propertyMap.Insert("rendererType",  "COLOR");
  propertyMap.Insert("mixColor",  testColor);

  Visual visual = factory.CreateVisual(propertyMap);
  DALI_TEST_CHECK( visual );

  Actor actor = Actor::New();
  TestVisualRender( application, actor, visual );

  Vector4 actualValue(Vector4::ZERO);
  TestGlAbstraction& gl = application.GetGlAbstraction();
  DALI_TEST_CHECK( gl.GetUniformValue<Vector4>( "mixColor", actualValue ) );
  DALI_TEST_EQUALS( actualValue, testColor, TEST_LOCATION );

  END_TEST;
}

int UtcDaliVisualFactoryGetColorVisual2(void)
{
  ToolkitTestApplication application;
  tet_infoline( "UtcDaliVisualFactoryGetColorVisual2: Request color visual with a Vector4" );

  VisualFactory factory = VisualFactory::Get();
  DALI_TEST_CHECK( factory );

  Vector4 testColor( 1.f, 0.5f, 0.3f, 0.2f );
  Dali::Property::Map map;
  map[ "rendererType" ] = "COLOR";
  map[ "mixColor" ] = testColor;
  Visual visual = factory.CreateVisual( map );
  DALI_TEST_CHECK( visual );

  Actor actor = Actor::New();
  TestVisualRender( application, actor, visual );

  Vector4 actualValue(Vector4::ZERO);
  TestGlAbstraction& gl = application.GetGlAbstraction();
  DALI_TEST_CHECK( gl.GetUniformValue<Vector4>( "mixColor", actualValue ) );
  DALI_TEST_EQUALS( actualValue, testColor, TEST_LOCATION );

  visual.SetOffStage( actor );
  DALI_TEST_CHECK( actor.GetRendererCount() == 0u );

  END_TEST;
}

int UtcDaliVisualFactoryGetBorderVisual1(void)
{
  ToolkitTestApplication application;
  tet_infoline( "UtcDaliVisualFactoryGetBorderVisual1:  Request border visual with a Property::Map" );

  VisualFactory factory = VisualFactory::Get();
  DALI_TEST_CHECK( factory );

  Property::Map propertyMap;
  Vector4 testColor( 1.f, 0.5f, 0.3f, 0.2f );
  float testSize = 5.f;
  propertyMap.Insert("rendererType",  "BORDER");
  propertyMap.Insert("borderColor",  testColor);
  propertyMap.Insert("borderSize",  testSize);

  Visual visual = factory.CreateVisual(propertyMap);
  DALI_TEST_CHECK( visual );

  Actor actor = Actor::New();
  actor.SetSize(200.f, 200.f);
  Stage::GetCurrent().Add( actor );
  visual.SetSize(Vector2(200.f, 200.f));
  visual.SetOnStage( actor );

  DALI_TEST_CHECK( actor.GetRendererCount() == 1u );
  int blendMode = actor.GetRendererAt(0u).GetProperty<int>( Renderer::Property::BLEND_MODE );
  DALI_TEST_EQUALS( static_cast<BlendingMode::Type>(blendMode), BlendingMode::ON, TEST_LOCATION );

  TestGlAbstraction& gl = application.GetGlAbstraction();

  application.SendNotification();
  application.Render(0);

  Vector4 actualColor(Vector4::ZERO);
  DALI_TEST_CHECK( gl.GetUniformValue<Vector4>( "borderColor", actualColor ) );
  DALI_TEST_EQUALS( actualColor, testColor, TEST_LOCATION );

  float actualSize = 0.f;
  DALI_TEST_CHECK( gl.GetUniformValue<float>( "borderSize", actualSize ) );
  DALI_TEST_EQUALS( actualSize, testSize, TEST_LOCATION );

  visual.SetOffStage( actor );
  DALI_TEST_CHECK( actor.GetRendererCount() == 0u );

  END_TEST;
}

int UtcDaliVisualFactoryGetBorderVisual2(void)
{
  ToolkitTestApplication application;
  tet_infoline( "UtcDaliVisualFactoryGetBorderVisual2:  Request border visual with a borderSize and a borderColor" );

  VisualFactory factory = VisualFactory::Get();
  DALI_TEST_CHECK( factory );

  Vector4 testColor( 1.f, 0.5f, 0.3f, 1.f );
  float testSize = 5.f;

  Dali::Property::Map propertyMap;
  propertyMap[ "rendererType" ] = "BORDER";
  propertyMap[ "borderColor"  ] = testColor;
  propertyMap[ "borderSize"   ] = testSize;
  Visual visual = factory.CreateVisual( propertyMap );
  DALI_TEST_CHECK( visual );

  Actor actor = Actor::New();
  actor.SetSize(200.f, 200.f);
  Stage::GetCurrent().Add( actor );
  visual.SetSize(Vector2(200.f, 200.f));
  visual.SetOnStage( actor );

  DALI_TEST_CHECK( actor.GetRendererCount() == 1u );

  TestGlAbstraction& gl = application.GetGlAbstraction();

  application.SendNotification();
  application.Render(0);

  int blendMode = actor.GetRendererAt(0u).GetProperty<int>( Renderer::Property::BLEND_MODE );
  DALI_TEST_EQUALS( static_cast<BlendingMode::Type>(blendMode), BlendingMode::AUTO, TEST_LOCATION );

  Vector4 actualColor(Vector4::ZERO);
  DALI_TEST_CHECK( gl.GetUniformValue<Vector4>( "borderColor", actualColor ) );
  DALI_TEST_EQUALS( actualColor, testColor, TEST_LOCATION );

  float actualSize = 0.f;
  DALI_TEST_CHECK( gl.GetUniformValue<float>( "borderSize", actualSize ) );
  DALI_TEST_EQUALS( actualSize, testSize, TEST_LOCATION );

  visual.SetOffStage( actor );

  // enable the anti-aliasing
  Dali::Property::Map map;
  map[ "rendererType" ] = "BORDER";
  map[ "borderColor"  ] = testColor;
  map[ "borderSize"   ] = testSize;
  map[ "antiAliasing"   ] = true;
  visual = factory.CreateVisual( map );
  visual.SetOnStage( actor );

  application.SendNotification();
  application.Render(0);
  blendMode = actor.GetRendererAt(0u).GetProperty<int>( Renderer::Property::BLEND_MODE );
  DALI_TEST_EQUALS( static_cast<BlendingMode::Type>(blendMode), BlendingMode::ON, TEST_LOCATION );

  END_TEST;
}

int UtcDaliVisualFactoryGetLinearGradientVisual(void)
{
  ToolkitTestApplication application;
  tet_infoline("UtcDaliVisualFactoryGetRadialGradientVisual");

  VisualFactory factory = VisualFactory::Get();
  DALI_TEST_CHECK( factory );

  Property::Map propertyMap;
  propertyMap.Insert("rendererType",  "GRADIENT");

  Vector2 start(-1.f, -1.f);
  Vector2 end(1.f, 1.f);
  propertyMap.Insert("startPosition", start);
  propertyMap.Insert("endPosition", end);
  propertyMap.Insert("spreadMethod", "REPEAT");

  Property::Array stopOffsets;
  stopOffsets.PushBack( 0.2f );
  stopOffsets.PushBack( 0.8f );
  propertyMap.Insert("stopOffset", stopOffsets);

  Property::Array stopColors;
  stopColors.PushBack( Color::RED );
  stopColors.PushBack( Color::GREEN );
  propertyMap.Insert("stopColor", stopColors);

  Visual visual = factory.CreateVisual(propertyMap);
  DALI_TEST_CHECK( visual );

  // A lookup texture is generated and pass to shader as sampler
  Actor actor = Actor::New();
  TestVisualRender( application, actor, visual, 1u );

  visual.SetOffStage( actor );
  DALI_TEST_CHECK( actor.GetRendererCount() == 0u );

  END_TEST;
}

int UtcDaliVisualFactoryGetRadialGradientVisual(void)
{
  ToolkitTestApplication application;
  tet_infoline("UtcDaliVisualFactoryGetRadialGradientVisual");

  VisualFactory factory = VisualFactory::Get();
  DALI_TEST_CHECK( factory );

  Property::Map propertyMap;
  propertyMap.Insert("rendererType",  "GRADIENT");

  Vector2 center(100.f, 100.f);
  float radius = 100.f;
  propertyMap.Insert("units",  "USER_SPACE");
  propertyMap.Insert("center",  center);
  propertyMap.Insert("radius",  radius);

  Property::Array stopOffsets;
  stopOffsets.PushBack( 0.0f );
  stopOffsets.PushBack( 1.f );
  propertyMap.Insert("stopOffset",   stopOffsets);

  Property::Array stopColors;
  stopColors.PushBack( Color::RED );
  stopColors.PushBack( Color::GREEN );
  propertyMap.Insert("stopColor",   stopColors);

  Visual visual = factory.CreateVisual(propertyMap);
  DALI_TEST_CHECK( visual );

  // A lookup texture is generated and pass to shader as sampler
  Actor actor = Actor::New();
  TestVisualRender( application, actor, visual, 1u );

  Matrix3 alignMatrix( radius, 0.f, 0.f, 0.f, radius, 0.f, center.x, center.y, 1.f );
  alignMatrix.Invert();

  Matrix3 actualValue( Matrix3::IDENTITY );
  TestGlAbstraction& gl = application.GetGlAbstraction();
  DALI_TEST_CHECK( gl.GetUniformValue<Matrix3>( "uAlignmentMatrix", actualValue ) );
  DALI_TEST_EQUALS( actualValue, alignMatrix, Math::MACHINE_EPSILON_100, TEST_LOCATION );

  END_TEST;
}

int UtcDaliVisualFactoryDefaultOffsetsGradientVisual(void)
{
  ToolkitTestApplication application;
  tet_infoline("UtcDaliVisualFactoryGetRadialGradientVisual");

  VisualFactory factory = VisualFactory::Get();
  DALI_TEST_CHECK( factory );

  Property::Map propertyMap;
  propertyMap.Insert("rendererType",  "GRADIENT");

  Vector2 start(-1.f, -1.f);
  Vector2 end(1.f, 1.f);
  propertyMap.Insert("startPosition", start);
  propertyMap.Insert("endPosition", end);
  propertyMap.Insert("spreadMethod", "REPEAT");

  Property::Array stopColors;
  stopColors.PushBack( Color::RED );
  stopColors.PushBack( Color::GREEN );
  propertyMap.Insert("stopColor", stopColors);

  Visual visual = factory.CreateVisual(propertyMap);
  DALI_TEST_CHECK( visual );

  // A lookup texture is generated and pass to shader as sampler
  Actor actor = Actor::New();
  TestVisualRender( application, actor, visual, 1u );

  visual.SetOffStage( actor );
  DALI_TEST_CHECK( actor.GetRendererCount() == 0u );

  END_TEST;
}

int UtcDaliVisualFactoryGetImageVisual1(void)
{
  ToolkitTestApplication application;
  tet_infoline( "UtcDaliVisualFactoryGetImageVisual1: Request image renderer with a Property::Map" );

  VisualFactory factory = VisualFactory::Get();
  DALI_TEST_CHECK( factory );

  Property::Map propertyMap;
  propertyMap.Insert( "rendererType",  "IMAGE" );
  propertyMap.Insert( "url",  TEST_IMAGE_FILE_NAME );

  Visual visual = factory.CreateVisual( propertyMap );
  DALI_TEST_CHECK( visual );

  Actor actor = Actor::New();
  // For tesing the LoadResourceFunc is called, a big image size should be set, so the atlasing is not applied.
  // Image with a size smaller than 512*512 will be uploaded as a part of the atlas.

  const int width=512;
  const int height=513;
  TestGlAbstraction& gl = application.GetGlAbstraction();
  TraceCallStack& textureTrace = gl.GetTextureTrace();
  textureTrace.Enable(true);

  Integration::Bitmap* bitmap = Integration::Bitmap::New( Integration::Bitmap::BITMAP_2D_PACKED_PIXELS, ResourcePolicy::OWNED_DISCARD );
  bitmap->GetPackedPixelsProfile()->ReserveBuffer( Pixel::RGBA8888, width, height,width, height );

  TestVisualRender( application, actor, visual, 1u,
                             ImageDimensions(width, height),
                             Integration::ResourcePointer( bitmap ) );

  DALI_TEST_EQUALS( textureTrace.FindMethod("BindTexture"), true, TEST_LOCATION );

  visual.SetOffStage( actor );
  DALI_TEST_CHECK( actor.GetRendererCount() == 0u );

  END_TEST;
}

int UtcDaliVisualFactoryGetImageVisual2(void)
{
  ToolkitTestApplication application;
  tet_infoline( "UtcDaliVisualFactoryGetImageVisual2: Request image renderer with an image handle" );

  VisualFactory factory = VisualFactory::Get();
  DALI_TEST_CHECK( factory );

  Image image = ResourceImage::New(TEST_IMAGE_FILE_NAME);
  Visual visual = factory.CreateVisual( image );

  Actor actor = Actor::New();
  // For tesing the LoadResourceFunc is called, a big image size should be set, so the atlasing is not applied.
  // Image with a size smaller than 512*512 will be uploaded as a part of the atlas.

  const int width=512;
  const int height=513;

  Integration::Bitmap* bitmap = Integration::Bitmap::New( Integration::Bitmap::BITMAP_2D_PACKED_PIXELS, ResourcePolicy::OWNED_DISCARD );
  bitmap->GetPackedPixelsProfile()->ReserveBuffer( Pixel::RGBA8888, width, height,width, height );

  TestGlAbstraction& gl = application.GetGlAbstraction();
  TraceCallStack& textureTrace = gl.GetTextureTrace();
  textureTrace.Enable(true);

  TestVisualRender( application, actor, visual, 1u,
                             ImageDimensions(width, height),
                             Integration::ResourcePointer(bitmap) );

  DALI_TEST_EQUALS( textureTrace.FindMethod("BindTexture"), true, TEST_LOCATION );

  END_TEST;
}

int UtcDaliVisualFactoryGetNPatchVisual1(void)
{
  ToolkitTestApplication application;
  tet_infoline( "UtcDaliVisualFactoryGetNPatchVisual1: Request 9-patch renderer with a Property::Map" );

  VisualFactory factory = VisualFactory::Get();
  DALI_TEST_CHECK( factory );

  const unsigned int ninePatchImageHeight = 18;
  const unsigned int ninePatchImageWidth = 28;
  StretchRanges stretchRangesX;
  stretchRangesX.PushBack( Uint16Pair( 2, 3 ) );
  StretchRanges stretchRangesY;
  stretchRangesY.PushBack( Uint16Pair( 4, 5 ) );
  Integration::ResourcePointer ninePatchResource = CustomizeNinePatch( application, ninePatchImageWidth, ninePatchImageHeight, stretchRangesX, stretchRangesY );

  Property::Map propertyMap;
  propertyMap.Insert( "rendererType",  "IMAGE" );
  propertyMap.Insert( "url",  TEST_NPATCH_FILE_NAME );
  {
    tet_infoline( "whole grid" );
    Visual visual = factory.CreateVisual( propertyMap );
    DALI_TEST_CHECK( visual );

    Actor actor = Actor::New();

    TestGlAbstraction& gl = application.GetGlAbstraction();
    TraceCallStack& textureTrace = gl.GetTextureTrace();
    textureTrace.Enable(true);

    TestVisualRender( application, actor, visual, 1u,
                               ImageDimensions(ninePatchImageWidth, ninePatchImageHeight),
                               ninePatchResource );

    DALI_TEST_EQUALS( textureTrace.FindMethod("BindTexture"), true, TEST_LOCATION );
  }

  propertyMap.Insert( "borderOnly",  true );
  {
    tet_infoline( "border only" );
    Visual visual = factory.CreateVisual( propertyMap );
    DALI_TEST_CHECK( visual );

    Actor actor = Actor::New();

    TestGlAbstraction& gl = application.GetGlAbstraction();
    TraceCallStack& textureTrace = gl.GetTextureTrace();
    textureTrace.Enable(true);

    TestVisualRender( application, actor, visual, 1u,
                               ImageDimensions(ninePatchImageWidth, ninePatchImageHeight),
                               ninePatchResource );

    DALI_TEST_EQUALS( textureTrace.FindMethod("BindTexture"), true, TEST_LOCATION );
  }

  END_TEST;
}

int UtcDaliVisualFactoryGetNPatchVisual2(void)
{
  ToolkitTestApplication application;
  tet_infoline( "UtcDaliVisualFactoryGetNPatchVisual2: Request n-patch renderer with a Property::Map" );

  VisualFactory factory = VisualFactory::Get();
  DALI_TEST_CHECK( factory );

  const unsigned int ninePatchImageWidth = 18;
  const unsigned int ninePatchImageHeight = 28;
  StretchRanges stretchRangesX;
  stretchRangesX.PushBack( Uint16Pair( 2, 3 ) );
  stretchRangesX.PushBack( Uint16Pair( 5, 7 ) );
  stretchRangesX.PushBack( Uint16Pair( 12, 15 ) );
  StretchRanges stretchRangesY;
  stretchRangesY.PushBack( Uint16Pair( 4, 5 ) );
  stretchRangesY.PushBack( Uint16Pair( 8, 12 ) );
  stretchRangesY.PushBack( Uint16Pair( 15, 16 ) );
  stretchRangesY.PushBack( Uint16Pair( 25, 27 ) );
  Integration::ResourcePointer ninePatchResource = CustomizeNinePatch( application, ninePatchImageWidth, ninePatchImageHeight, stretchRangesX, stretchRangesY );

  Property::Map propertyMap;
  propertyMap.Insert( "rendererType",  "IMAGE" );
  propertyMap.Insert( "url",  TEST_NPATCH_FILE_NAME );
  {
    Visual visual = factory.CreateVisual( propertyMap );
    DALI_TEST_CHECK( visual );

    Actor actor = Actor::New();
    TestGlAbstraction& gl = application.GetGlAbstraction();
    TraceCallStack& textureTrace = gl.GetTextureTrace();
    textureTrace.Enable(true);

    TestVisualRender( application, actor, visual, 1u,
                               ImageDimensions(ninePatchImageWidth, ninePatchImageHeight),
                               ninePatchResource );


    DALI_TEST_EQUALS( textureTrace.FindMethod("BindTexture"), true, TEST_LOCATION );

    visual.SetOffStage( actor );
    DALI_TEST_CHECK( actor.GetRendererCount() == 0u );
  }

  propertyMap.Insert( "borderOnly",  true );
  {
    tet_infoline( "border only" );
    Visual visual = factory.CreateVisual( propertyMap );
    DALI_TEST_CHECK( visual );

    TestGlAbstraction& gl = application.GetGlAbstraction();
    TraceCallStack& textureTrace = gl.GetTextureTrace();
    textureTrace.Enable(true);
    Actor actor = Actor::New();
    TestVisualRender( application, actor, visual, 1u,
                               ImageDimensions(ninePatchImageWidth, ninePatchImageHeight),
                               ninePatchResource );


    DALI_TEST_EQUALS( textureTrace.FindMethod("BindTexture"), true, TEST_LOCATION );

    visual.SetOffStage( actor );
    DALI_TEST_CHECK( actor.GetRendererCount() == 0u );
  }

  END_TEST;
}

int UtcDaliVisualFactoryGetNPatchVisual3(void)
{
  ToolkitTestApplication application;
  tet_infoline( "UtcDaliVisualFactoryGetNPatchVisual3: Request 9-patch renderer with an image url" );

  VisualFactory factory = VisualFactory::Get();
  DALI_TEST_CHECK( factory );

  const unsigned int ninePatchImageHeight = 18;
  const unsigned int ninePatchImageWidth = 28;
  StretchRanges stretchRangesX;
  stretchRangesX.PushBack( Uint16Pair( 2, 3 ) );
  StretchRanges stretchRangesY;
  stretchRangesY.PushBack( Uint16Pair( 4, 5 ) );
  Integration::ResourcePointer ninePatchResource = CustomizeNinePatch( application, ninePatchImageWidth, ninePatchImageHeight, stretchRangesX, stretchRangesY );

  Visual visual = factory.CreateVisual( TEST_NPATCH_FILE_NAME, ImageDimensions() );
  DALI_TEST_CHECK( visual );

  Actor actor = Actor::New();

  TestGlAbstraction& gl = application.GetGlAbstraction();
  TraceCallStack& textureTrace = gl.GetTextureTrace();
  textureTrace.Enable(true);

  TestVisualRender( application, actor, visual, 1u,
                             ImageDimensions(ninePatchImageWidth, ninePatchImageHeight),
                             ninePatchResource );

  DALI_TEST_EQUALS( textureTrace.FindMethod("BindTexture"), true, TEST_LOCATION );

  END_TEST;
}

int UtcDaliVisualFactoryGetNPatchVisual4(void)
{
  ToolkitTestApplication application;
  tet_infoline( "UtcDaliVisualFactoryGetNPatchVisual4: Request n-patch visual with an image url" );

  VisualFactory factory = VisualFactory::Get();
  DALI_TEST_CHECK( factory );

  const unsigned int ninePatchImageHeight = 18;
  const unsigned int ninePatchImageWidth = 28;
  StretchRanges stretchRangesX;
  stretchRangesX.PushBack( Uint16Pair( 2, 3 ) );
  stretchRangesX.PushBack( Uint16Pair( 5, 7 ) );
  stretchRangesX.PushBack( Uint16Pair( 12, 15 ) );
  StretchRanges stretchRangesY;
  stretchRangesY.PushBack( Uint16Pair( 4, 5 ) );
  stretchRangesY.PushBack( Uint16Pair( 8, 12 ) );
  stretchRangesY.PushBack( Uint16Pair( 15, 16 ) );
  stretchRangesY.PushBack( Uint16Pair( 25, 27 ) );
  Integration::ResourcePointer ninePatchResource = CustomizeNinePatch( application, ninePatchImageWidth, ninePatchImageHeight, stretchRangesX, stretchRangesY );

  Visual visual = factory.CreateVisual( TEST_NPATCH_FILE_NAME, ImageDimensions() );
  DALI_TEST_CHECK( visual );

  Actor actor = Actor::New();

  TestGlAbstraction& gl = application.GetGlAbstraction();
  TraceCallStack& textureTrace = gl.GetTextureTrace();
  textureTrace.Enable(true);

  TestVisualRender( application, actor, visual, 1u,
                             ImageDimensions(ninePatchImageWidth, ninePatchImageHeight),
                             ninePatchResource );

  DALI_TEST_EQUALS( textureTrace.FindMethod("BindTexture"), true, TEST_LOCATION );

  END_TEST;
}

int UtcDaliVisualFactoryGetNPatchVisualN1(void)
{
  //This should still load but display an error image

  ToolkitTestApplication application;
  tet_infoline( "UtcDaliVisualFactoryGetNPatchVisualN: Request n-patch visual with an invalid image url" );

  VisualFactory factory = VisualFactory::Get();
  DALI_TEST_CHECK( factory );

  Visual visual = factory.CreateVisual( "ERROR.9.jpg", ImageDimensions() );
  DALI_TEST_CHECK( visual );

  Actor actor = Actor::New();

  //The testkit still has to load a bitmap for the broken renderer image
  Integration::Bitmap* bitmap = Integration::Bitmap::New(Integration::Bitmap::BITMAP_2D_PACKED_PIXELS, ResourcePolicy::OWNED_DISCARD);
  bitmap->GetPackedPixelsProfile()->ReserveBuffer( Pixel::RGBA8888, 100, 100, 100, 100 );

  TestGlAbstraction& gl = application.GetGlAbstraction();
  TraceCallStack& textureTrace = gl.GetTextureTrace();
  textureTrace.Enable(true);

  TestVisualRender( application, actor, visual, 1u,
                             ImageDimensions(),
                             Integration::ResourcePointer(bitmap) );

  DALI_TEST_EQUALS( textureTrace.FindMethod("BindTexture"), true, TEST_LOCATION );

  END_TEST;
}

int UtcDaliVisualFactoryGetNPatchVisualN2(void)
{
  //This should still load but display an error image

  ToolkitTestApplication application;
  tet_infoline( "UtcDaliVisualFactoryGetNPatchVisualN: Request n-patch visual with an invalid Property::Map" );

  VisualFactory factory = VisualFactory::Get();
  DALI_TEST_CHECK( factory );

  Property::Map propertyMap;
  propertyMap.Insert( "rendererType",  111 );
  propertyMap.Insert( "url",  "ERROR.9.jpg" );

  Visual visual = factory.CreateVisual( propertyMap );
  DALI_TEST_CHECK( visual );

  Actor actor = Actor::New();

  //The testkit still has to load a bitmap for the broken renderer image
  Integration::Bitmap* bitmap = Integration::Bitmap::New(Integration::Bitmap::BITMAP_2D_PACKED_PIXELS, ResourcePolicy::OWNED_DISCARD);
  bitmap->GetPackedPixelsProfile()->ReserveBuffer( Pixel::RGBA8888, 100, 100, 100, 100 );

  TestGlAbstraction& gl = application.GetGlAbstraction();
  TraceCallStack& textureTrace = gl.GetTextureTrace();
  textureTrace.Enable(true);

  TestVisualRender( application, actor, visual, 1u,
                             ImageDimensions(),
                             Integration::ResourcePointer(bitmap) );

  DALI_TEST_EQUALS( textureTrace.FindMethod("BindTexture"), true, TEST_LOCATION );

  END_TEST;
}

int UtcDaliVisualFactoryGetSvgVisual(void)
{
  ToolkitTestApplication application;
  tet_infoline( "UtcDaliVisualFactoryGetSvgVisual: Request svg visual with a svg url" );

  VisualFactory factory = VisualFactory::Get();
  Visual visual = factory.CreateVisual( TEST_SVG_FILE_NAME, ImageDimensions() );
  DALI_TEST_CHECK( visual );

  TestGlAbstraction& gl = application.GetGlAbstraction();
  TraceCallStack& textureTrace = gl.GetTextureTrace();
  textureTrace.Enable(true);

  Actor actor = Actor::New();
  actor.SetSize( 200.f, 200.f );
  Stage::GetCurrent().Add( actor );
  visual.SetSize( Vector2(200.f, 200.f) );
  visual.SetOnStage( actor );
  application.SendNotification();
  application.Render();

  DALI_TEST_CHECK( actor.GetRendererCount() == 1u );

  EventThreadCallback* eventTrigger = EventThreadCallback::Get();
  CallbackBase* callback = eventTrigger->GetCallback();

  eventTrigger->WaitingForTrigger( 1 );// waiting until the svg image is rasterized.
  CallbackBase::Execute( *callback );

  DALI_TEST_CHECK( actor.GetRendererCount() == 1u );

  // waiting for the resource uploading
  application.SendNotification();
  application.Render();

  DALI_TEST_EQUALS( textureTrace.FindMethod("BindTexture"), true, TEST_LOCATION );

  END_TEST;
}

//Creates a mesh renderer from the given propertyMap and tries to load it on stage in the given application.
//This is expected to succeed, which will then pass the test.
void MeshVisualLoadsCorrectlyTest( Property::Map& propertyMap, ToolkitTestApplication& application )
{
  VisualFactory factory = VisualFactory::Get();
  DALI_TEST_CHECK( factory );

  //Create a mesh visual.
  Visual visual = factory.CreateVisual( propertyMap );
  DALI_TEST_CHECK( visual );

  //Create an actor on stage to house the visual.
  Actor actor = Actor::New();
  actor.SetSize( 200.f, 200.f );
  Stage::GetCurrent().Add( actor );
  visual.SetSize( Vector2( 200.f, 200.f ) );
  visual.SetOnStage( actor );

  //Ensure set on stage.
  DALI_TEST_EQUALS( actor.GetRendererCount(), 1u, TEST_LOCATION );

  //Attempt to render to queue resource load requests.
  application.SendNotification();
  application.Render( 0 );

  //Tell the platform abstraction that the required resources have been loaded.
  TestPlatformAbstraction& platform = application.GetPlatform();
  platform.SetAllResourceRequestsAsLoaded();

  //Render again to upload the now-loaded textures.
  application.SendNotification();
  application.Render( 0 );

  Matrix testScaleMatrix;
  testScaleMatrix.SetIdentityAndScale( Vector3( 1.0, -1.0, 1.0 ) );
  Matrix actualScaleMatrix;

  //Test to see if the object has been successfully loaded.
  DALI_TEST_CHECK( application.GetGlAbstraction().GetUniformValue<Matrix>( "uObjectMatrix", actualScaleMatrix ) );
  DALI_TEST_EQUALS( actualScaleMatrix, testScaleMatrix, Math::MACHINE_EPSILON_100, TEST_LOCATION );

  //Finish by setting off stage, and ensuring this was successful.
  visual.SetOffStage( actor );
  DALI_TEST_EQUALS( actor.GetRendererCount(), 0u, TEST_LOCATION );
}

//Creates a mesh visual from the given propertyMap and tries to load it on stage in the given application.
//This is expected to fail, which will then pass the test.
void MeshVisualDoesNotLoadCorrectlyTest( Property::Map& propertyMap, ToolkitTestApplication& application )
{
  VisualFactory factory = VisualFactory::Get();
  DALI_TEST_CHECK( factory );

  //Create a mesh visual.
  Visual visual = factory.CreateVisual( propertyMap );
  DALI_TEST_CHECK( visual );

  //Create an actor on stage to house the visual.
  Actor actor = Actor::New();
  actor.SetSize( 200.f, 200.f );
  Stage::GetCurrent().Add( actor );
  visual.SetSize( Vector2( 200.f, 200.f ) );
  visual.SetOnStage( actor );

  //Ensure set on stage.
  DALI_TEST_EQUALS( actor.GetRendererCount(), 1u, TEST_LOCATION );

  //Attempt to render to queue resource load requests.
  application.SendNotification();
  application.Render( 0 );

  //Tell the platform abstraction that the required resources have been loaded.
  TestPlatformAbstraction& platform = application.GetPlatform();
  platform.SetAllResourceRequestsAsLoaded();

  //Render again to upload the now-loaded textures.
  application.SendNotification();
  application.Render( 0 );

  //Test to see if the object has not been loaded, as expected.
  Matrix scaleMatrix;
  DALI_TEST_CHECK( !application.GetGlAbstraction().GetUniformValue<Matrix>( "uObjectMatrix", scaleMatrix ) );

  //Finish by setting off stage, and ensuring this was successful.
  visual.SetOffStage( actor );
  DALI_TEST_EQUALS( actor.GetRendererCount(), 0u, TEST_LOCATION );
}

//Test if mesh loads correctly when supplied with only the bare minimum requirements, an object file.
int UtcDaliVisualFactoryGetMeshVisual1(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;

  tet_infoline( "UtcDaliVisualFactoryGetMeshVisual1:  Request mesh visual with a valid object file only" );


  //Set up visual properties.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType",  "MESH" );
  propertyMap.Insert( "objectUrl", TEST_OBJ_FILE_NAME );

  //Test to see if mesh loads correctly.
  MeshVisualLoadsCorrectlyTest( propertyMap, application );

  END_TEST;
}


//Test if mesh loads correctly when supplied with an object file as well as a blank material file and images directory.
int UtcDaliVisualFactoryGetMeshVisual2(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;

  tet_infoline( "UtcDaliVisualFactoryGetMeshVisual2:  Request mesh visual with blank material file and images directory" );

  //Set up visual properties.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType", "MESH" );
  propertyMap.Insert( "objectUrl", TEST_OBJ_FILE_NAME );
  propertyMap.Insert( "materialUrl", "" );
  propertyMap.Insert( "texturesPath", "" );

  //Test to see if mesh loads correctly.
  MeshVisualLoadsCorrectlyTest( propertyMap, application );

  END_TEST;
}

//Test if mesh loads correctly when supplied with all main parameters, an object file, a material file and a directory location.
int UtcDaliVisualFactoryGetMeshVisual3(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;

  tet_infoline( "UtcDaliVisualFactoryGetMeshVisual3:  Request mesh visual with all parameters correct" );

  //Set up visual properties.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType", "MESH" );
  propertyMap.Insert( "objectUrl", TEST_OBJ_FILE_NAME );
  propertyMap.Insert( "materialUrl", TEST_MTL_FILE_NAME );
  propertyMap.Insert( "texturesPath", TEST_RESOURCE_DIR "/" );

  //Test to see if mesh loads correctly.
  MeshVisualLoadsCorrectlyTest( propertyMap, application );

  END_TEST;
}

//Test if mesh visual can load a correctly supplied mesh without a normal map or gloss map in the material file.
int UtcDaliVisualFactoryGetMeshVisual4(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;

  tet_infoline( "UtcDaliVisualFactoryGetMeshVisual4:  Request mesh visual with diffuse texture but not normal or gloss." );


  //Set up visual properties.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType", "MESH" );
  propertyMap.Insert( "objectUrl", TEST_OBJ_FILE_NAME );
  propertyMap.Insert( "materialUrl", TEST_SIMPLE_MTL_FILE_NAME );
  propertyMap.Insert( "texturesPath", TEST_RESOURCE_DIR "/" );

  //Test to see if mesh loads correctly.
  MeshVisualLoadsCorrectlyTest( propertyMap, application );


  END_TEST;
}

//Test if mesh visual can load when made to use diffuse textures only.
int UtcDaliVisualFactoryGetMeshVisual5(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;

  tet_infoline( "UtcDaliVisualFactoryGetMeshVisual5:  Request mesh visual and make it only use diffuse textures." );

  //Set up visual properties.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType", "MESH" );
  propertyMap.Insert( "objectUrl", TEST_OBJ_FILE_NAME );
  propertyMap.Insert( "materialUrl", TEST_MTL_FILE_NAME );
  propertyMap.Insert( "texturesPath", TEST_RESOURCE_DIR "/" );
  propertyMap.Insert( "shaderType", "DIFFUSE_TEXTURE" );

  //Test to see if mesh loads correctly.
  MeshVisualLoadsCorrectlyTest( propertyMap, application );


  END_TEST;
}

//Test if mesh visual can load when made to not use the supplied textures.
int UtcDaliVisualFactoryGetMeshVisual6(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;

  tet_infoline( "UtcDaliVisualFactoryGetMeshVisual6:  Request mesh visual and make it not use any textures." );

  //Set up visual properties.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType", "MESH" );
  propertyMap.Insert( "objectUrl", TEST_OBJ_FILE_NAME );
  propertyMap.Insert( "materialUrl", TEST_MTL_FILE_NAME );
  propertyMap.Insert( "texturesPath", TEST_RESOURCE_DIR "/" );
  propertyMap.Insert( "shaderType", "TEXTURELESS" );

  //Test to see if mesh loads correctly.
  MeshVisualLoadsCorrectlyTest( propertyMap, application );

  END_TEST;
}
//Test if mesh visual loads correctly when light position is manually set.
int UtcDaliVisualFactoryGetMeshVisual7(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;


  tet_infoline( "UtcDaliVisualFactoryGetMeshVisual7:  Request mesh visual with custom light position." );

  //Set up visual properties.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType", "MESH" );
  propertyMap.Insert( "objectUrl", TEST_OBJ_FILE_NAME );
  propertyMap.Insert( "materialUrl", TEST_MTL_FILE_NAME );
  propertyMap.Insert( "texturesPath", TEST_RESOURCE_DIR "/" );
  propertyMap.Insert( "lightPosition", Vector3( 0.0, 1.0, 2.0 ) );

  //Test to see if mesh loads correctly.
  MeshVisualLoadsCorrectlyTest( propertyMap, application );

  END_TEST;
}

//Test if mesh visual loads correctly when supplied an object file without face normals or texture points.
//Note that this notably tests object loader functionality.
int UtcDaliVisualFactoryGetMeshVisual8(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;

  tet_infoline( "UtcDaliVisualFactoryGetMeshVisual5:  Request mesh visual with normal-less object file." );

  //Set up visual properties.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType", "MESH" );
  propertyMap.Insert( "objectUrl", TEST_SIMPLE_OBJ_FILE_NAME );
  propertyMap.Insert( "materialUrl", TEST_MTL_FILE_NAME );
  propertyMap.Insert( "texturesPath", TEST_RESOURCE_DIR "/" );

  //Test to see if mesh loads correctly.
  MeshVisualLoadsCorrectlyTest( propertyMap, application );

  END_TEST;
}

//Test if mesh visual handles the case of lacking an object file.
int UtcDaliVisualFactoryGetMeshVisualN1(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;

  tet_infoline( "UtcDaliVisualFactoryGetMeshVisualN1:  Request mesh visual without object file" );

  //Set up visual properties.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType", "MESH" );
  propertyMap.Insert( "materialUrl", TEST_MTL_FILE_NAME );
  propertyMap.Insert( "texturesPath", TEST_RESOURCE_DIR "/" );

  //Test to see if mesh doesn't load with these properties, as expected.
  MeshVisualDoesNotLoadCorrectlyTest( propertyMap, application );


  END_TEST;
}

//Test if mesh visual handles the case of being passed invalid material and images urls.
int UtcDaliVisualFactoryGetMeshVisualN2(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;

  tet_infoline( "UtcDaliVisualFactoryGetMeshVisualN2:  Request mesh visual with invalid material and images urls" );

  //Set up visual properties.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType", "MESH" );
  propertyMap.Insert( "objectUrl", TEST_OBJ_FILE_NAME );
  propertyMap.Insert( "materialUrl", "invalid" );
  propertyMap.Insert( "texturesPath", "also invalid" );

  //Test to see if mesh doesn't load with these properties, as expected.
  MeshVisualDoesNotLoadCorrectlyTest( propertyMap, application );


  END_TEST;
}

//Test if mesh visual handles the case of being passed an invalid object url
int UtcDaliVisualFactoryGetMeshVisualN3(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;
  tet_infoline( "UtcDaliVisualFactoryGetMeshVisualN3:  Request mesh visual with invalid object url" );


  //Set up visual properties.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType", "MESH" );
  propertyMap.Insert( "objectUrl", "invalid" );
  propertyMap.Insert( "materialUrl", TEST_MTL_FILE_NAME );
  propertyMap.Insert( "texturesPath", TEST_RESOURCE_DIR "/" );

  //Test to see if mesh doesn't load with these properties, as expected.
  MeshVisualDoesNotLoadCorrectlyTest( propertyMap, application );

  END_TEST;
}

//Creates a primitive visual with the given property map and tests to see if it correctly loads in the given application.
void TestPrimitiveVisualWithProperties( Property::Map& propertyMap, ToolkitTestApplication& application )
{
  VisualFactory factory = VisualFactory::Get();
  DALI_TEST_CHECK( factory );

  //Create a primitive visual.
  Visual visual = factory.CreateVisual( propertyMap );
  DALI_TEST_CHECK( visual );

  //Create an actor on stage to house the visual.
  Actor actor = Actor::New();
  actor.SetSize( 200.f, 200.f );
  Stage::GetCurrent().Add( actor );
  visual.SetSize( Vector2( 200.f, 200.f ) );
  visual.SetOnStage( actor );

  //Ensure set on stage.
  DALI_TEST_EQUALS( actor.GetRendererCount(), 1u, TEST_LOCATION );

  //Tell test application to load the visual.
  application.SendNotification();
  application.Render(0);

  Matrix testScaleMatrix;
  testScaleMatrix.SetIdentityAndScale( Vector3( 1.0, -1.0, 1.0 ) );
  Matrix actualScaleMatrix;

  //Test to see if the object has been successfully loaded.
  DALI_TEST_CHECK( application.GetGlAbstraction().GetUniformValue<Matrix>( "uObjectMatrix", actualScaleMatrix ) );
  DALI_TEST_EQUALS( actualScaleMatrix, testScaleMatrix, Math::MACHINE_EPSILON_100, TEST_LOCATION );

  //Finish by setting off stage, and ensuring this was successful.
  visual.SetOffStage( actor );
  DALI_TEST_EQUALS( actor.GetRendererCount(), 0u, TEST_LOCATION );
}

//Test if primitive shape loads correctly when supplied with only the bare minimum requirements, the shape to use.
int UtcDaliVisualFactoryGetPrimitiveVisual1(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;

  tet_infoline( "UtcDaliVisualFactoryGetPrimitiveVisual1:  Request primitive visual with a shape only" );

  //Set up visual properties.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType", "PRIMITIVE" );
  propertyMap.Insert( "shape", "CUBE" );

  //Test to see if shape loads correctly.
  TestPrimitiveVisualWithProperties( propertyMap, application );

  END_TEST;
}

//Test if primitive shape loads correctly when supplied with all possible parameters
int UtcDaliVisualFactoryGetPrimitiveVisual2(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;

  tet_infoline( "UtcDaliVisualFactoryGetPrimitiveVisual2:  Request primitive visual with everything" );

  //Set up visual properties.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType", "PRIMITIVE" );
  propertyMap.Insert( "shape", "CUBE" );
  propertyMap.Insert( "shapeColor", Vector4( 0.5, 0.5, 0.5, 1.0 ) );
  propertyMap.Insert( "slices", 10 );
  propertyMap.Insert( "stacks", 20 );
  propertyMap.Insert( "scaleTopRadius", 30.0f );
  propertyMap.Insert( "scaleBottomRadius", 40.0f );
  propertyMap.Insert( "scaleHeight", 50.0f );
  propertyMap.Insert( "scaleRadius", 60.0f );
  propertyMap.Insert( "bevelPercentage", 0.7f );
  propertyMap.Insert( "bevelSmoothness", 0.8f );
  propertyMap.Insert( "lightPosition", Vector3( 0.9, 1.0, 1.1 ) );

  //Test to see if shape loads correctly.
  TestPrimitiveVisualWithProperties( propertyMap, application );

  END_TEST;
}

//Test if primitive shape loads a sphere correctly.
int UtcDaliVisualFactoryGetPrimitiveVisual3(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;

  tet_infoline( "UtcDaliVisualFactoryGetPrimitiveVisual3:  Request primitive visual to display a sphere" );

  //Set up visual properties.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType", "PRIMITIVE" );
  propertyMap.Insert( "shape", "SPHERE" );
  propertyMap.Insert( "shapeColor", Vector4( 0.5, 0.5, 0.5, 1.0 ) );
  propertyMap.Insert( "slices", 10 );
  propertyMap.Insert( "stacks", 20 );

  //Test to see if shape loads correctly.
  TestPrimitiveVisualWithProperties( propertyMap, application );

  END_TEST;
}

//Test if primitive shape loads a conic section correctly.
int UtcDaliVisualFactoryGetPrimitiveVisual4(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;

  tet_infoline( "UtcDaliVisualFactoryGetPrimitiveVisual4:  Request primitive visual to display a conic section" );

  //Set up visual properties.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType", "PRIMITIVE" );
  propertyMap.Insert( "shape", "CONICAL_FRUSTRUM" );
  propertyMap.Insert( "shapeColor", Vector4( 0.5, 0.5, 0.5, 1.0 ) );
  propertyMap.Insert( "slices", 10 );
  propertyMap.Insert( "scaleTopRadius", 30.0f );
  propertyMap.Insert( "scaleBottomRadius", 40.0f );
  propertyMap.Insert( "scaleHeight", 50.0f );

  //Test to see if shape loads correctly.
  TestPrimitiveVisualWithProperties( propertyMap, application );

  END_TEST;
}

//Test if primitive shape loads a bevelled cube correctly.
int UtcDaliVisualFactoryGetPrimitiveVisual5(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;

  tet_infoline( "UtcDaliVisualFactoryGetPrimitiveVisual5:  Request primitive visual to display a bevelled cube" );

  //Set up visual properties.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType", "PRIMITIVE" );
  propertyMap.Insert( "shape", "BEVELLED_CUBE" );
  propertyMap.Insert( "shapeColor", Vector4( 0.5, 0.5, 0.5, 1.0 ) );
  propertyMap.Insert( "bevelPercentage", 0.7f );

  //Test to see if shape loads correctly.
  TestPrimitiveVisualWithProperties( propertyMap, application );

  END_TEST;
}

//Test if primitive shape loads an octahedron correctly.
int UtcDaliVisualFactoryGetPrimitiveVisual6(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;

  tet_infoline( "UtcDaliVisualFactoryGetPrimitiveVisual6:  Request primitive visual to display an octahedron" );

  //Set up visual properties.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType", "PRIMITIVE" );
  propertyMap.Insert( "shape", "OCTAHEDRON" );
  propertyMap.Insert( "shapeColor", Vector4( 0.5, 0.5, 0.5, 1.0 ) );

  //Test to see if shape loads correctly.
  TestPrimitiveVisualWithProperties( propertyMap, application );

  END_TEST;
}

//Test if primitive shape loads a cone correctly.
int UtcDaliVisualFactoryGetPrimitiveVisual7(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;

  tet_infoline( "UtcDaliVisualFactoryGetPrimitiveVisual7:  Request primitive visual to display a cone" );

  //Set up visual properties.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType", "PRIMITIVE" );
  propertyMap.Insert( "shape", "CONE" );
  propertyMap.Insert( "shapeColor", Vector4( 0.5, 0.5, 0.5, 1.0 ) );
  propertyMap.Insert( "slices", 10 );
  propertyMap.Insert( "scaleTopRadius", 30.0f );
  propertyMap.Insert( "scaleHeight", 50.0f );

  //Test to see if shape loads correctly.
  TestPrimitiveVisualWithProperties( propertyMap, application );

  END_TEST;
}

//Test if primitive shape loads correctly when light position is manually set.
int UtcDaliVisualFactoryGetPrimitiveVisual8(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;

  tet_infoline( "UtcDaliVisualFactoryGetPrimitiveVisual8:  Request primitive visual with set light position" );

  //Set up visual properties.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType", "PRIMITIVE" );
  propertyMap.Insert( "shape", "SPHERE" );
  propertyMap.Insert( "shapeColor", Vector4( 0.5, 0.5, 0.5, 1.0 ) );
  propertyMap.Insert( "lightPosition", Vector3( 0.0, 1.0, 2.0 ) );

  //Test to see if shape loads correctly.
  TestPrimitiveVisualWithProperties( propertyMap, application );

  END_TEST;
}

//Test if primitive shape renderer handles the case of not being passed a specific shape to use.
int UtcDaliVisualFactoryGetPrimitiveVisualN1(void)
{
  //Set up test application first, so everything else can be handled.
  ToolkitTestApplication application;

  tet_infoline( "UtcDaliVisualFactoryGetPrimitiveVisualN1:  Request primitive visual without shape" );

  //Set up visual properties, without supplying shape.
  Property::Map propertyMap;
  propertyMap.Insert( "rendererType", "PRIMITIVE" );

  //Test to see if shape loads regardless of missing input.
  TestPrimitiveVisualWithProperties( propertyMap, application );

  END_TEST;
}
