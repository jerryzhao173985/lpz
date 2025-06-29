#ifndef __MULTI_CONTROLLER_H
#define __MULTI_CONTROLLER_H


#include <cstdio>
#include "abstractcontroller.h"

#include <vector>
#include <list>

typedef std::vector<AbstractController*> ControllerContainer;

/**
 * class for multi controller
 */
class MultiController : public AbstractController {
public:

  MultiController();

  /** initialisation of the controller with the given sensor/ motornumber
      Must be called before use.
  */
  virtual void init(int sensornumber, int motornumber);

  /// returns the name of the object (with version number)
  //  virtual constparamkey getName() const {return name; }
  virtual paramkey getName() const override;

  /// @return Number of sensors the controller was initialised with or 0 if not initialised
  virtual int getSensorNumber() const override;


  /// @return Number of motors the controller was initialised with or 0 if not initialised
  virtual int getMotorNumber() const override;

  /** performs one step (includes learning).
      Calculates motor commands from sensor inputs.
      @param sensor sensors inputs scaled to [-1,1]
      @param sensornumber length of the sensor array
      @param motor motors outputs. MUST have enough space for motor values!
      @param motornumber length of the provided motor array
  */
  virtual void step(const sensor* sensors, int sensornumber,
                    motor* motors, int motornumber);
  /** performs one step without learning.
      @see step
  */
  virtual void stepNoLearning(const sensor* , int number_sensors,
                              motor* , int number_motors);

  /** The list of the names of all internal parameters given by getInternalParams().
      @param: keylist (do NOT free it! It is a pointer to an internal structure)
      @return: length of the lists
   */
  virtual std::list<iparamkey> getInternalParamNames() const override;

  /** The list of the names of all internal parameters given by getInternalParams().
   */
  virtual list<iparamval> getInternalParams() const override;

  virtual paramval getParam(const paramkey& key, bool traverseChildren=true) const;
  virtual bool setParam(const paramkey& key, paramval val, bool traverseChildren=true);
  virtual paramlist getParamList() const;

  /** Initialises the registers the given callback functions.
      @param handling() is called every step that the camera gets new position
      and view.
  */
//   virtual void setCameraHandling(void (*handling)());
//  virtual void setCameraHandling() const override;


  ControllerContainer& get_controller_container() const;
  ControllerContainer::iterator get_active_controller();
  void set_active_controller(ControllerContainer::iterator it);

  void next_controller();

protected:
  //string name;
  ControllerContainer controller_container;
  ControllerContainer::iterator it_active_controller;

  /*
  int t = 0;
  int number_sensors = 0;
  int number_motors = 0;
  int cameraHandlingDefined = 0;

  paramval velocity;
  paramval leftRightShift;
  paramval decreaseFactorVelocity;
  paramval decreaseFactorShift;
  */

};

#endif
