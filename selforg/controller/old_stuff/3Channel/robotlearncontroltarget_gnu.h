#include "robotlearncontroltarget.h"
#include "plotlog.h"

template <int N, int buffer_size=1> class RobotLearnControlTarget_Gnu{

protected:
         int t = 0;

        inline std::string explicit intToStr(int value)
        {
          std::ostringstream stream;
          stream << value;
          return stream.str();
        }

public:
    PlotLog<std::string> plotlog;
    PlotLog<std::string> plotlog_x;
    PlotLog<std::string> plotlog_a;


  RobotLearnControlTarget_Gnu():
     : RobotLearnControlTarget<N,buffer_size> (),
    plotlog("c.dat"),plotlog_x("x.dat"),plotlog_a("a.dat"), t(0) {

        for (int i=0; i<N; ++i) {
            plotlog.addChannel("h["+intToStr(i)+"]");
          plotlog_x.addChannel("x["+intToStr(i)+"]");
          plotlog_x.addChannel("y["+intToStr(i)+"]");
          for (int j=0; j<N; ++j) {
            plotlog.addChannel("c["+intToStr(i)+"]["+intToStr(j)+"]");
            plotlog_a.addChannel("a["+intToStr(i)+"]["+intToStr(j)+"]");
          }
        }
        plotlog.command("set zeroaxis");
        //plotlog.print_names_list();

        plotlog_x.command("set zeroaxis");
        //plotlog_x.print_names_list();

        plotlog_a.command("set zeroaxis");
        //plotlog_a.print_names_list();

        t=0;

  };


  /// make step (calculate controller outputs and learn controller)
  virtual void makeStep(double *x_, double *y_) {

    RobotLearnControlTarget<N,buffer_size>::makeStep(x_, y_);
    //std::cout<<y_[0]<<__PLACEHOLDER_20__<<y_[1]<<std::endl;

    for (int i=0; i<N; ++i) {
      plotlog.putData("h["+intToStr(i)+"]",h[i]);
      plotlog_x.putData("x["+intToStr(i)+"]",x_[i]);
      plotlog_x.putData("y["+intToStr(i)+"]",y_[i]);
      for (int j=0; j<N; ++j) {
        plotlog.putData("c["+intToStr(i)+"]["+intToStr(j)+"]",C[i][j]);
        plotlog_a.putData("a["+intToStr(i)+"]["+intToStr(j)+"]",A[i][j]);
      }
    }

    ++t;
    //if (t%100) return;

    plotlog.plot();
//    plotlog.print();

    plotlog_x.plot();
//    plotlog_x.print();

    plotlog_a.plot();
//    plotlog_a.print();

  };
/*
  virtual void makeStepWithoutLearning(double *x_, double *y_) {

  RobotLearnControl<N,buffer_size>::makeStepWithoutLearning(x_, y_);
    __PLACEHOLDER_55__




    for (int i=0; i<N; ++i) {
      plotlog.putData(__PLACEHOLDER_34__+intToStr(i)+__PLACEHOLDER_35__,h[i]);
      plotlog_x.putData(__PLACEHOLDER_36__+intToStr(i)+__PLACEHOLDER_37__,x_[i]);
      plotlog_x.putData(__PLACEHOLDER_38__+intToStr(i)+__PLACEHOLDER_39__,y_[i]);
      for (int j=0; j<N; ++j) {
        plotlog.putData(__PLACEHOLDER_40__+intToStr(i)+__PLACEHOLDER_41__+intToStr(j)+__PLACEHOLDER_42__,C[i][j]);
        plotlog_a.putData(__PLACEHOLDER_43__+intToStr(i)+__PLACEHOLDER_44__+intToStr(j)+__PLACEHOLDER_45__,A[i][j]);
      }
    }
    plotlog.plot();
    plotlog.print();

    plotlog_x.plot();
    plotlog_x.print();

    plotlog_a.plot();
    plotlog_a.print();

  };
*/
};

