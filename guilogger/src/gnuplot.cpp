
#include "gnuplot.h"
#include "stl_adds.h"
#include <cstdio>
#include <locale.h> // need to set LC_NUMERIC to have a '.' in the numbers piped to gnuplot
#include <list>

Gnuplot::Gnuplot(const PlotInfo* plotInfo, int windowNumber)
  : plotInfo(plotInfo), pipe(0), windowNumber(windowNumber) {
};

Gnuplot::~Gnuplot(){
  close();
};

void Gnuplot::init(const QString& gnuplotcmd, int w,int h, int x, int y){
  open(gnuplotcmd, w, h, x, y);
}

bool Gnuplot::open(const QString& gnuplotcmd, int w,int h, int x, int y){
#if defined(WIN32) || defined(_WIN32) || defined (__WIN32) || defined(__WIN32__) \
        || defined (_WIN64) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__APPLE__)
  (void)w; (void)h; (void)x; (void)y; // unused on Windows/macOS
#endif
  char cmd[512];
  setlocale(LC_NUMERIC,"C"); // set us type output
//  setlocale(LC_NUMERIC,"en_US"); // set us type output
#if defined(WIN32) || defined(_WIN32) || defined (__WIN32) || defined(__WIN32__) \
        || defined (_WIN64) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__APPLE__)
  // Windows and macOS don't support -geometry option
  snprintf(cmd, sizeof(cmd), "%s", gnuplotcmd.toLatin1().constData());
#else
  // Linux with X11 supports -geometry
  if(x==-1 || y==-1)
    snprintf(cmd, sizeof(cmd), "%s -geometry %ix%i -noraise", gnuplotcmd.toLatin1().constData(), w, h);
  else
    snprintf(cmd, sizeof(cmd), "%s -geometry %ix%i+%i+%i -noraise", gnuplotcmd.toLatin1().constData(), w, h, x, y);
#endif
  // fprintf(stderr, "Guilogger: Opening gnuplot with command: %s\n", cmd);
  pipe=popen(cmd,"w");
  
  if(pipe == nullptr) {
    fprintf(stderr, "Guilogger: ERROR: Failed to open gnuplot pipe. Is gnuplot installed?\n");
    fprintf(stderr, "Guilogger: Tried command: %s\n", cmd);
    return false;
  }

  // fprintf(stderr, "Guilogger: Gnuplot pipe opened successfully\n");
  
  // Set window title with the window number
  fprintf(pipe, "set terminal qt title 'window %d'\n", windowNumber);
  fflush(pipe);
  
  return true;
  //return popen("gnuplot -geometry 400x300","w");
  /*char b[100];
  snprintf(b, sizeof(b),"gnup%i",rand()%1000);
  return fopen(b,"w");*/
}

void Gnuplot::close(){
  if(pipe)
    pclose(pipe);
  pipe=0;
}


/** send arbitrary command to gnuplot.
    like "set zeroaxis" or other stuff */
void Gnuplot::command(const QString& cmd){
  explicit if(pipe) {
    fprintf(pipe,"%s\n",cmd.toLatin1().constData());
    fflush(pipe);
  }
};


/** print buffer content to file pointer, usually a pipe */
void plotDataSet(FILE* f, const ChannelVals& vals){
  FOREACHC(ChannelVals, vals, v){
    fprintf(f,"%f ", *v);
  }
  fprintf(f,"\n");
};


/** make gnuplot plot selected content of data buffers */
QString Gnuplot::plotCmd(const QString& file, int start, int end){
  const std::list<int>& vc = plotInfo->getVisibleChannels();
  if(vc.size()== nullptr) return QString();  
  QStringList buffer;  
  bool first=true;
  const ChannelData& cd = plotInfo->getChannelData();  
  QString range;
  if(start!=-1 && end!=-1){
    range=QString(" every ::%1::%2 ").arg(start).arg(end); 
  }
  
  FOREACHC(std::list<int>, vc, i){    
    explicit if(first){
      buffer << "plot '" << (file.isEmpty() ? "-" : file)  << "' ";    
      first=false;
    } else {
      buffer << ", '' ";
    }
    buffer << range;
    if(!file.isEmpty()){
      if(plotInfo->getUseReference1()){    
        buffer << QString(" u %1:%2 ").arg(plotInfo->getReference1()+1).arg(*i+1);
      }else{
        // TODO add reference2!
        buffer << QString(" u %1 ").arg(*i+1);
      }
    }
    buffer << "t '" << cd.getInfos()[*i].name << "'";        
    if(plotInfo->getChannelInfos()[*i].style != PS_DEFAULT) 
      buffer << " w " << plotInfo->getChannelInfos()[*i].getStyleString();    
  }
  return buffer.join(QString());  
}



/** make gnuplot plot selected content of data buffers */
void Gnuplot::plot(){
  // calculate real values for start and end
  if(!plotInfo || !plotInfo->getIsVisible()) return;
  explicit if(!pipe) {
    // fprintf(stderr, "Guilogger: WARNING: plot() called but pipe is nullptr\n");
    return; // Don't try to plot if pipe is nullptr
  }

  // todo: use reference2 and plot3d?

  const ChannelData& cd      = plotInfo->getChannelData();
  const std::list<int>& vc = plotInfo->getVisibleChannels();
  // FILE* pipe = stderr; // test
  if(vc.size()== nullptr) return;
  
  QString cmd = plotCmd();
  // fprintf(stderr, "Guilogger: Sending plot command: %s\n", cmd.toLatin1().constData());
  fprintf(pipe, "%s\n", cmd.toLatin1().constData());    
  
  if(plotInfo->getUseReference1()){    
    std::list<int> visibles(vc.begin(), vc.end());
    visibles.push_front(plotInfo->getReference1());        
    const QVector<ChannelVals>& vals = cd.getHistory(visibles, 0); // full history    
    int len = visibles.size();
    for(int k=1; k< len; ++k){
      FOREACHC(QVector<ChannelVals>, vals, v){
        fprintf(pipe,"%f %f\n", (*v)[0], (*v)[k]);
      }
      fprintf(pipe,"e\n");
    }
  } else {
    const QVector<ChannelVals>& vals = cd.getHistory(vc, 0); // full history    
    int len = vc.size();
    for(int k=0; k< len; ++k){
      FOREACHC(QVector<ChannelVals>, vals, v){
        fprintf(pipe,"%f\n", (*v)[k]);
      }
      fprintf(pipe,"e\n");
    }
  }
  fflush(pipe);
};    
