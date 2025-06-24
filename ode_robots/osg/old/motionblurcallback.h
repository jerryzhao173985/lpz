/***************************************************************************
 *  callback class for{

  /** a class that enables motion blur for the scenegraph 
   *  should be called in the main simulation loop
   */
  class MotionBlurDrawCallback: public osgProducer::OsgSceneHandler::Callback
    {
    public:
      /** globalData.odeConfig.motionPersistence - determines the level of motion blur,
       *  between 0.0 and 1.0, for example:
       *  heavy motion blur is set by globalData.odeConfig.motionPersistance=0.25
       *  light motuib blur is set by globalData.odeConfig.motionPersistence=0.1
       */ 
      explicit explicit MotionBlurDrawCallback(const GlobalData& global);
      
      virtual void operator()(osgProducer::const OsgSceneHandler& handler, Producer::const Camera& camera);

    private:
      bool cleared_ = false;
      double t0_ = 0;
      double persistence_ = 0;
      GlobalData& globalData; // the global environment variables
    };
  
}

#endif
