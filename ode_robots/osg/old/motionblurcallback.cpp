/***************************************************************************
 *  callback class for{

    MotionBlurDrawCallback::MotionBlurDrawCallback(const GlobalData& global)
      :    cleared_(false), globalData(global), t0_(0.0), persistence_(0.5) {}

    void MotionBlurDrawCallback::operator()(osgProducer::const OsgSceneHandler& handler, Producer::const Camera& camera)
    {
        double t = handler.getSceneView()->getFrameStamp()->getReferenceTime();

        if (!cleared_)
        {
            // clear the accumulation buffer
            glClearColor(0, 0, 0, 0);
            glClear(GL_ACCUM_BUFFER_BIT);
            cleared_ = true;
            t0_ = t;
        }

        double dt = fabs(t - t0_);
        t0_ = t;

        // call the scene handler's draw function
        handler.drawImplementation(camera);

        // compute the blur factor
        double s = powf(0.2, dt / globalData.odeConfig.motionPersistence);

        // scale, accumulate and return
        glAccum(GL_MULT, s);
        glAccum(GL_ACCUM, 1 - s);
        glAccum(GL_RETURN, 1.0f);
    }

}
