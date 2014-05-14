#ifndef __DALI_TOOLKIT_INTERNAL_SCROLL_OVERSHOOT_INDICATOR_H__
#define __DALI_TOOLKIT_INTERNAL_SCROLL_OVERSHOOT_INDICATOR_H__

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

#include <dali/dali.h>
#include <dali-toolkit/public-api/shader-effects/bouncing-effect.h>

namespace Dali
{

namespace Toolkit
{

namespace Internal
{
class Scrollable;
class ScrollOvershootEffect;
class ScrollOvershootEffectGradient;
class ScrollOvershootEffectRipple;
typedef IntrusivePtr<ScrollOvershootEffect> ScrollOvershootEffectPtr;
typedef IntrusivePtr<ScrollOvershootEffectGradient> ScrollOvershootEffectGradientPtr;
typedef IntrusivePtr<ScrollOvershootEffectRipple> ScrollOvershootEffectRipplePtr;

struct ScrollOvershootIndicator : public Dali::RefObject
{
public:

  /**
   * ScrollOvershootIndicator constructor.
   * @param[in] scrollView reference to ScrollView implementation
   * or horizontally (false)
   */
  ScrollOvershootIndicator( Scrollable& scrollable);

  /**
   * Virtual destructor
   */
  virtual ~ScrollOvershootIndicator();

  /**
   * Enables and disables the indicator
   * &param[in] enable true to enable, false to disable
   */
  void Enable(bool enable);

  /**
   * Resets the indicator
   */
  void Reset();

  /**
   * Create an initialized ScrollOvershootIndicator
   * @param[in] scrollView reference to ScrollView implementation
   * or horizontally (false)
   * @return A pointer to the created ScrollOvershootIndicator.
   */
  static ScrollOvershootIndicator* New( Scrollable& scrollable);

private:
  Scrollable& mScrollable;                                ///< Internal::Scrollable object
  ScrollOvershootEffectPtr mEffectX;                      ///< effect used for x-axis/horizontal display
  ScrollOvershootEffectPtr mEffectY;                      ///< effect used for y-axis/vertical display
};

/**
 * ScrollOvershootEffect is a derivable class, designed to allow the application programmer to create their own
 * overshoot effect and apply it with minimal implementation required
 */
struct ScrollOvershootEffect : public Dali::RefObject
{
public:
  /**
   * Create a new overshoot effect, passing in whether it is vertical or horizontal
   *
   * @param[in] vertical whether this effect is a vertical or horizontal one
   */
  ScrollOvershootEffect(bool vertical);

  /**
   * Virtual destructor
   */
  virtual ~ScrollOvershootEffect() {}

  /**
   * Returns if this is a vertical or horizontal overhoot effect
   *
   * @return true or false
   */
  inline bool IsVertical() { return mVertical; }

  /**
   * Applies the indicator effect, all derived effects must implement this function
   *
   * @param[in] scrollable the scrollable object to apply this effect to
   */
  virtual void Apply(Scrollable& scrollable) = 0;

  /**
   * Removes the indicator effect, all derived effects must implement this function
   *
   * @param[in] scrollable the scrollable object to remove this effect from
   */
  virtual void Remove(Scrollable& scrollable) = 0;

  /**
   * Resets this overshoot effect
   */
  virtual void Reset() = 0;

  /**
   * Updates the constraints used for the overshoot effect
   *
   * @param[in] scrollable the container for the overshoot effect
   */
  virtual void UpdateConstraints(Actor& scrollable) {}

  /**
   * Sets up property notifications for overshoot values
   *
   * @param[in] scrollable the container for the overshoot effect
   */
  virtual void SetPropertyNotifications(Actor& scrollable) {}

private:
  bool mVertical;                      ///< whether this is a vertical/horizontal effect
};

/**
 * ScrollOvershootEffectRipple creates an animated bounce effect at the end of the scrollable area if the user
 * attempts to scroll past it
 */
struct ScrollOvershootEffectRipple : public ScrollOvershootEffect, public ConnectionTracker
{
public:
  /**
   * Create a new gradient overshoot effect, passing in whether it is vertical or horizontal
   */
  ScrollOvershootEffectRipple(bool vertical);

  /**
   * @copydoc ScrollOvershootEffect::Apply
   */
  virtual void Apply(Scrollable& scrollable);

  /**
   * @copydoc ScrollOvershootEffect::Remove
   */
  virtual void Remove(Scrollable& scrollable);

  /**
   * @copydoc ScrollOvershootEffect::Reset
   */
  virtual void Reset();

  /**
   * @copydoc ScrollOvershootEffect::UpdateConstraints(Actor& scrollable)
   */
  virtual void UpdateConstraints(Actor& scrollable);

  /**
   * @copydoc ScrollOvershootEffect::SetPropertyNotification
   */
  virtual void SetPropertyNotifications(Actor& scrollable);

  /**
   * Constrains the size of the gradient image
   * @param[in] current current position of the image actor
   * @param[in] parentSizeProperty size of the scrollable area so we can position image on the edge of it
   * @param[in] overshootProperty current overshoot amount for this indicator's axis
   * @return new position of the gradient image actor
   */
  Vector3 PositionConstraint(const Vector3& current, const PropertyInput& parentSizeProperty, const PropertyInput& overshootProperty);

  /**
   * Informs overshoot effect to update image position and to animate effect overshoot value for a
   * positive overshoot value from scrollview
   *
   * @param[in] source the property notification that triggered this callback
   */
  void OnPositiveOvershootNotification(PropertyNotification& source);

  /**
   * Informs overshoot effect to update image position and to animate effect overshoot value for a
   * negative overshoot value from scrollview
   *
   * @param[in] source the property notification that triggered this callback
   */
  void OnNegativeOvershootNotification(PropertyNotification& source);

  /**
   * Function to animate effect overshoot value either to -1.0f/1.0f or 0.0f
   *
   * @param[in] overshootAmount the amount to animate overshoot to [-1.0f,0.0f,1.0f]
   */
  void AnimateScrollOvershoot(float overshootAmount);

  /**
   * Connects to the animation finished signal of our overshoot animation
   *
   * @param[in] animation the animation instance that has finished
   */
  void OnOvershootAnimFinished(Animation& animation);

  /**
   * Creates a new ScrollOvershootEffectGradient objects and returns a pointer to it
   * @param[in] vertical whether to create a vertical(true) or horizontal effect
   * @return a pointer to the new effect
   */
  static ScrollOvershootEffectRipplePtr New( bool vertical );

private:

  float mMaxOvershootImageSize;            ///< maximum size of the image when overshoot value is 1.0f
  ImageActor mOvershootImage;              ///< the overshoot image...
  Animation mScrollOvershootAnimation;     ///< animation
  bool      mAnimatingOvershootOn;         ///< whether we are currently animating overshoot to 1.0f/-1.0f (on) or to 0.0f (off)
  bool      mAnimateOvershootOff;          ///< whether we are currently animating overshoot to 1.0f/-1.0f (on) or to 0.0f (off)
  int       mCanScrollPropertyIndex;       ///< property index to a property that informs indicator if it is needed
  BouncingEffect mRippleEffect;                 // the ripple vertex/fragment shader effect
  PropertyNotification mOvershootPositiveNotification; // stores the property notification used for positive overshoot values
  PropertyNotification mOvershootNegativeNotification; // stores the property notification used for negative overshoot values
  ActiveConstraint    mSizeConstraint;                 // active constraint handle used to store the image width constraint
  ActiveConstraint    mPositionConstraint;             // active constraint handle used to store the image position constraint
};

} // namespace Internal

} // namespace Toolkit

} // namespace Dali

#endif // __DALI_TOOLKIT_INTERNAL_SCROLL_OVERSHOOT_INDICATOR_H__