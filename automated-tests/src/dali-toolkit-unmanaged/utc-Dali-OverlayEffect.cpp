//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <iostream>
#include <stdlib.h>
#include <dali-toolkit-test-suite-utils.h>
#include <dali-toolkit/dali-toolkit.h>


using namespace Dali;
using namespace Dali::Toolkit;

void overlay_effect_startup(void)
{
  test_return_value = TET_UNDEF;
}

void overlay_effect_cleanup(void)
{
  test_return_value = TET_PASS;
}


int UtcDaliOverlayConstructor(void)
{
  ToolkitTestApplication application;

  BitmapImage image = CreateBitmapImage();

  Toolkit::OverlayEffect effect = Toolkit::OverlayEffect::New( image );
  DALI_TEST_CHECK( effect );

  ImageActor actor = ImageActor::New( image );
  actor.SetSize( 100.0f, 100.0f );
  actor.SetShaderEffect( effect );
  Stage::GetCurrent().Add( actor );

  application.SendNotification();
  application.Render();

  END_TEST;
}

int UtcDaliOverlayUninitializedEffect(void)
{
  ToolkitTestApplication application;

  Toolkit::OverlayEffect effect;

  try
  {
    BitmapImage image = CreateBitmapImage();

    // New() must be called to create a OverlayEffect or it wont be valid.
    effect.SetEffectImage( image );
    DALI_TEST_CHECK( false );
  }
  catch (Dali::DaliException& e)
  {
    // Tests that a negative test of an assertion succeeds
    tet_printf("Assertion %s failed at %s\n", e.mCondition.c_str(), e.mLocation.c_str());
    DALI_TEST_CHECK(!effect);
  }
  END_TEST;
}