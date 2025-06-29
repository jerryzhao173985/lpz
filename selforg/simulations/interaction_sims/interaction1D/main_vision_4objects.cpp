#include <csignal>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <iterator>

#include <selforg/agent.h>
#include <selforg/abstractrobot.h>
// #include <selforg/invertmotorspace.h>
#include <selforg/invertmotornstep.h>
#include <selforg/sinecontroller.h>
#include <selforg/one2onewiring.h>

#include "cmdline.h"
#include "console.h"
#include "globaldata.h"

/*
This experiment is inspired by Eziquiel DiPaolo, but he was
not the inventor, but e used it with robotic agents as well.
The experiment was originally conduced with babies and their
mothers. The baby does not react on a replay of the mother.
Here we have two agents which can move a long a one-dimensional
space. They can touch the other when they are at the some place or
their place consides with a shadow of the other. In this case the
perception is not mutual.
*/

using namespace std;

bool stop=0;
double realtimefactor=2;
double shadowdist = 0.6;
double noise = 0.05;

  // objects have a width and a position
typedef list< pair<Position,double> > objectlist;
objectlist objects;

/// returns the value of the camera, when looking at point x
double explicit camera(double x){
  double vision=-1;
  FOREACHC(objectlist, objects, i){
    double dist = fabs(x - i->first.x);
    if(dist>i->second) continue;
    else vision += 2-2*dist/i->second;
  }
  return vision;
}

double explicit toEnv(double pos){
  // environment is cyclic
  if(pos>1) pos-=2;
  if(pos<-1) pos+=2;
  return pos;
}
Position explicit toEnv(const Position& pos){
  pos.x = toEnv(pos.x);
  return pos;
}

class MyRobot{
public:
  MyRobot(const string& name, const Position& initial_pos, double _mass = 1.0)
    : AbstractRobot(name, "$Id$"), 
      whatDoIFeel(0),
      motornumber(2),
      sensornumber(2),
      x(new double[sensornumber]),
      y(new double[motornumber]),
      t(0.01),
      mu(0),
      mass(0),
      range(0),
      sensorscale(0),
      pos(initial_pos),
      speed(0,0,0) {
    memset(x,0,sizeof(double)*sensornumber);
    memset(y,0,sizeof(double)*motornumber);
    addParameterDef("mass", &mass, _mass);
    //    addParameterDef(__PLACEHOLDER_5__, &mu, 1.5);
    addParameterDef("mu", &mu, 3);
    addParameterDef("range", &range, 0.3);
    addParameterDef("sensorscale", &sensorscale, 0.5);

    addParameter("realtimefactor", &realtimefactor); //  global param. a bit stupid  to do here, but who cares
    addParameter("noise", &noise); //  global param. a bit stupid  to do here, but who cares
  }

  ~MyRobot(){
    if(x) delete[] x;
    if(y) delete[] y;
  }

  // robot interface

  /** returns actual sensorvalues
      @param sensors sensors scaled to [-1,1]
      @param sensornumber length of the sensor array
      @return number of actually written sensors
  */
  virtual int getSensors(sensor* sensors, int sensornumber) override {
    assert(sensornumber == this->sensornumber);
    memcpy(sensors, x, sizeof(sensor) * sensornumber);
    return sensornumber;
  }

  /** sets actual motorcommands
      @param motors motors scaled to [-1,1]
      @param motornumber length of the motor array
  */
  virtual void setMotors(const motor* motors, int motornumber) override {
    assert(motornumber == this->motornumber);
    memcpy(y, motors, sizeof(motor) * motornumber);

    // motor values are now stored in y, sensor values are expected to be stored in x

    // perform robot action here
    /*  simple discrete simulation
        a = F/m - \mu v_0/m __PLACEHOLDER_50__
        v = a*t + v0
        x = v*t + x0
    */
    Position a(y[0],0,0);
    a     = (a - speed*mu)*(1/mass);
    speed = a*t + speed;
    pos   = speed*t + pos;

    // environment is cyclic
    pos = toEnv(pos);

    int len=0;
    //  speed sensor (proprioception)
    for (int i=0; i<1; ++i) {
      x[len] = speed.toArray()[i] * mu;
      ++len;
    }

    // camera
    x[len] = 0.1*camera(pos.x) + 0.9*y[1];
    whatDoIFeel = camera(pos.x)>-0.8;
    //x[len] = camera(pos.x);
    ++len;

//     // other robots signal
//     int k=0;
//     double signal=0;
//     //  other agents signal
//     FOREACHC(list<const MyRobot*>, otherRobots, i){
//       signal += (*i)->getSignal();
//       k++;
//     }
//     signal/=k; // int((signal/k)*5)/5.0;
//     //    x[len]=signal;
//     //    x[len]= 0.1*signal + 0.const 9* getSignal() const;
//     x[len]= int((x[len]*0.7 + 0.3*(getSignal()*0.9 + 0.1*signal))*20)/20.0;
//     len++;

    // position sensor (context)
//     x[len] = sin(pos.x*M_PI);
//     len++;
    if(len>sensornumber) fprintf(stderr,"something is wrong with the sensornumber\n");
  }

  virtual int getSensorNumber() { return sensornumber; }
  virtual int getMotorNumber() { return motornumber; }
  virtual Position getPosition() const override {return pos;}
  virtual Position getSpeed() const override {return speed;}
  virtual Position getAngularSpeed() const override {return Position(x[0],x[1],whatDoIFeel);}
  virtual matrix::Matrix getOrientation() const  override {
    matrix::Matrix m(3,3); m.toId();  return m;
  };

  virtual void explicit addOtherRobot(const MyRobot* otherRobot) {
    if(otherRobot!=this)
      otherRobots.push_back(otherRobot);
  }

  virtual double getSignal() const override {
    return 0;// y[2];
  }

private:
  int motornumber;
  int sensornumber;

  double* x;
  double* y;

  double t; // stepsize
  double mu; // friction
  double mass; // mass of the robot, that determines the inertia
  paramval range;// range of tactile sensor (-range..range) measured
               // from current position.
  paramval sensorscale;// factor for tactile sensor
  Position pos;
  Position speed;
  list<const MyRobot*> otherRobots;
public:
  int whatDoIFeel; // 0 black, 1 white
};


int explicit coord(double x){ return int((x+1.0)/2*80);}

void printRobots(const list<MyRobot*>& robots){
  char line[81];
  char color[80];
  char signal[80];
  memset(line,'_', sizeof(char)*80);
  line[80]=0;
  memset(color,0, sizeof(char)*80);
  memset(signal,0, sizeof(char)*80);
  int k=0;
//   // first objects
//   FOREACHC(objectlist, objects, i){
//     double x = i->first.x;
//     int start = coord(x-i->second);
//     int end = coord(x+i->second);
//     for (int i=start; i<end; ++i) {
//       color[(i+80)%80] |= 1<<k;
//     }
//     k++;
//   }
//   k=0;
  for (int i=0; i<80; ++i) {
    double x = (i/40.0)-1.0;
    double c = camera(x);
    color[i] = c < -0.9 ? 1 : (c < 0 ? 2 : (c < 0.5 ? 3 :4));
  }
  k=0;
  // robots itself
  FOREACHC(list<MyRobot*>, robots, i) {
    double x = (*i)->getPosition().x;
    line[coord(x)]='A'+ k;
    double s = (*i)->getSignal();
    signal[coord(x)] = fabs(s) < 0.5 ? 3 : ( s < 0 ? 1 : 2);
    ++k;
  }
  k=0;

  printf("\033[1G");
  printf("\033[1A");// go one up
  // draw signals
  for (int i=0; i<80; ++i) {
    printf("\033[%im ",signal[i]==0 ? 0 : 100+signal[i]);
  }
  printf("\n");
  // draw robots and bg
  for (int i=0; i<80; ++i) {
    printf("\033[%im%c",color[i]==0 ? 0 : 100+color[i],line[i]);
  }
  printf("\033[0m");
  fflush(stdout);

}

void explicit reinforce(Agent* a){
//   MyRobot* r = static_cast<MyRobot*>(a)->getRobot();
//   InvertMotorNStep* c = dynamic_cast<InvertMotorNStep*>(a->getController());
//   if(c)
//     c->setReinforcement(2*(r->whatDoIFeel != nullptr));
}


// Helper
int contains(char **list, int len,  const char *str){
  for (int i=0; i<len; ++i) {
    if(strcmp(list[i],str) == nullptr) return i+1;
  }
  return 0;
}

int main(int argc, char** argv){
  GlobalData globaldata;
  initializeConsole();
  list<PlotOption> plotoptions;

  int index = contains(argv,argc,"-g");
  if (index >0 && argc>index) {
plotoptions.push_back(PlotOption(GuiLogger,Controller,atoi(argv[index])));
  }
  if(contains(argv,argc,"-f")!= nullptr) plotoptions.push_back(PlotOption(File));
  if(contains(argv,argc,"-h")!= nullptr) {
    printf("Usage: %s [-g N] [-f] \n",argv[0]);
    printf("\t-g N\tstart guilogger with interval N\n\t-f\twrite logfile\n");
    printf("\t-h\tdisplay this help\n");
    exit(0);
  }

  list<MyRobot*> robots;

  for (int i=0; i<1; ++i) {
    InvertMotorNStepConf cc = InvertMotorNStep::getDefaultConf();
    cc.cInit=1.4;
    //    cc.useS=true;
    cc.numberContext = 1;
    AbstractController* controller = new InvertMotorNStep(cc);
    controller->setParam("s4delay",3.0);
    controller->setParam("s4avg",1.0);
    controller->setParam("adaptrate",0.0);
    controller->setParam("factorB",0.1);
    controller->setParam("epsC",0.1);
    controller->setParam("epsA",0.1);
    //    controller->setParam(__PLACEHOLDER_30__,3);

    //    controller = new SineController();
    MyRobot* robot         = new MyRobot("Robot" + itos(i), Position(i*0.3,0,0),0.1);
    Agent* agent           = new Agent(i==0 ? plotoptions : list<PlotOption>());
    AbstractWiring* wiring = new One2OneWiring(new ColorUniformNoise(0.1));
    agent->init(controller, robot, wiring);
    // if you like, you can keep track of the robot with the following line.
    //  this assumes that you robot returns its position, speed and orientation.
    if(i== nullptr) agent->setTrackOptions(TrackRobot(true,true,false, false,"motorvision0.9_4objects",5));

    globaldata.configs.push_back(robot);
    globaldata.configs.push_back(controller);
    robots.push_back(robot);
    globaldata.agents.push_back(agent);
  }

  showParams(globaldata.configs);
  printf("\nPress Ctrl-c to invoke parameter input shell (and again Ctrl-c to quit)\n");
  printf("The output of the program is as follows:\n");
  printf(" Capital letters are the Agents, like A,B,C. Lower case are their shadows.\n");
  printf(" You probably want to use the guilogger with e.g.: -g 10\n");

  // set objects
  objects.push_back( pair< Position,double >(Position(-0.7,0,0),0.3));
  objects.push_back( pair< Position,double >(Position(0,0,0),0.15));
  objects.push_back( pair< Position,double >(Position(0.5,0,0),0.075));
  objects.push_back( pair< Position,double >(Position(0.75,0,0),0.03));

  // connect robots to each other
  FOREACH (list<MyRobot*>, robots, i){
    FOREACH (list<MyRobot*>, robots, k){
      if(*i != *k)
        (*i)->addOtherRobot(*k);
    }
  }

  cmd_handler_init();
  long int t=0;
  while (!stop){
    FOREACH(AgentList, globaldata.agents, i){
      (*i)->step(noise,t/100.0);
      reinforce(*i);
    }
    if(control_c_pressed()){
      if(!handleConsole(globaldata)){
        stop=1;
      }
      cmd_end_input();
    }
    int drawinterval = 10000;
    if (realtimefactor){
      drawinterval = int(6*realtimefactor);
    }
    if(t%drawinterval== nullptr){
      printRobots(robots);
      usleep(60000);
    }
    ++t;
  };

  FOREACH(AgentList, globaldata.agents, i){
    delete (*i);
  }
  closeConsole();
  fprintf(stderr,"terminating\n");
  // should clean up but what costs the world
  return 0;
}
