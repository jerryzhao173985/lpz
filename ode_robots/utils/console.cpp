/***************************************************************************
 *   Copyright (C) 2005-2011 LpzRobots development team                    *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Frank Guettler <guettler at informatik dot uni-leipzig dot de        *
 *    Frank Hesse    <frank at nld dot ds dot mpg dot de>                  *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                         *
 ***************************************************************************/


#include <cstdio>
#include <cstdlib>
#include <sys/ioctl.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <iostream>
#include <iomanip>

// Define whitespace macro if not provided by readline
#ifndef whitespace
#define whitespace(c) ((c) == ' ' || (c) == '\t')
#endif

#include <vector>
#include <sstream>
#include <string>
#include <selforg/stl_adds.h>
#include <selforg/abstractcontroller.h>
#include <selforg/configurable.h>
#include "globaldata.h"
#include "odeagent.h"
#include "abstractground.h"

using namespace std;

namespace lpzrobots {


typedef bool (*commandfunc_t)(const GlobalData& globalData, char *, char *);
/* The names of functions that actually do the manipulation.  parameter: global data, entire line, arg */
bool com_list (const GlobalData& globalData, char *, char *);
bool com_show (const GlobalData& globalData, char *, char *);
bool com_store (const GlobalData& globalData, char *, char *);
bool com_load (const GlobalData& globalData, char *, char *);
bool com_storecfg (const GlobalData& globalData, char *, char *);
bool com_loadcfg (const GlobalData& globalData, char *, char *);
bool com_contrs (const GlobalData& globalData, char *, char *);
bool com_set (const GlobalData& globalData, char *, char *);
bool com_help (const GlobalData& globalData, char *, char *);
bool com_quit (const GlobalData& globalData, char *, char *);

/* A structure which contains information on the commands this program
   can understand. */

typedef struct {
  const char *name;                   /* User printable name of the function. */
  commandfunc_t func;           /* Function to call to do the job. */
  const char *doc;                    /* Documentation for this function.  */
} COMMAND;

COMMAND commands[] = {
  { "param=val",  com_set, "sets PARAM of all objects to VAL" },
  { "help", com_help, "Display this text" },
  { "?",     com_help, "Synonym for `help'" },
  { "list", com_list, "Lists all configurables and agents" },
  { "ls",   com_list, "Synonym for `list'" },
  { "set",  com_set, "syntax: set [OBJECTID] PARAM=VAL; sets parameter of OBJECTID (or of all objects) to value" },
  { "store", com_store, "stores object. Syntax: store AGENTID FILE, see list" },
  { "load", com_load, "loads object. Syntax: load AGENTID FILE, see list" },
  { "storecfg", com_storecfg, "Store key-values pairs. Syntax: storecfg CONFIGID FILE" },
  { "loadcfg", com_loadcfg, "Load key-values pairs. Syntax: CONFIGID FILE" },
  { "contrs", com_contrs, "Stores the contours of all playgrounds to FILE" },
  { "show", com_show, "[OBJECTID]: Lists parameters of OBJECTID or of all objects (if no id given)" },
  { "view", com_show, "Synonym for `show'" },
  { "quit", com_quit, "Quit program" },
  { static_cast<char*>(nullptr), static_cast<commandfunc_t>(nullptr), static_cast<char*>(nullptr) }
};

typedef std::list<std::string> ParameterList;
ParameterList parameters; // used for completion

/* Forward declarations. */
char * stripwhite (char *string);
COMMAND *find_command (char *name);
bool execute_line (const GlobalData& globalData, char *line);
int valid_argument ( const char *caller, const char *arg);

int _quit_request=false;

void printConfigs(const std::list<::Configurable*>& configs)
{

  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  for(auto c = configs.begin(); c != configs.end(); ++c){
    (*c)->print(stdout, 0, w.ws_col-2, true);
  }
}

void printConfig(const ::Configurable* config)
{
  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  if(config) config->print(stdout, 0, w.ws_col-2, true);
}



char* dupstr (const char* s){
  char *r;

  r = static_cast<char*>(malloc (strlen (s) + 1));
  strcpy (r, s);
  return (r);
}

// duplicates string and adde an = sign
char* dupstrpluseq (const char* s){
  char *r;
  int len = strlen (s);
  r = static_cast<char*>(malloc (strlen (s) + 2));
  strcpy (r, s);
  r[len]= '=';
  r[len+1]= 0;
  return (r);
}

vector<string> splitstring(string s){
  istringstream split(s); //splitting the lines
  string word;
  vector<string> rv;
  while(split >> word) {
    if(word.compare(" ")!=0 && word.compare("  ")!= 0)
      rv.push_back(word);
  }
  return rv;
}

bool handleConsole(const GlobalData& globalData){
  char *line, *s;

  //  initialize_readline ();       /* Bind our completer. */
  // move to beginning of line (clear the ^C)
  std::cout << "\033[1G" << "Type: ? for help or press TAB\n";
  // collect parameters for completion
  parameters.clear();
  for (const auto& i : globalData.globalconfigurables){
    if(i)
      parameters += i->getAllParamNames();
  }

  line = readline ("> ");

  if (!line)
    return true;

  /* Remove leading and trailing whitespace from the line.
     Then, if there is anything left, add it to the history list
     and execute it. */
  s = stripwhite (line);
  if (*s) {
    add_history (s);
    bool success = execute_line (globalData,s);
    if(success){
      for(OdeAgentList::const_iterator i = globalData.agents.begin(); i != globalData.agents.end(); ++i){
        (*i)->writePlotComment(s);
      }
    }
  }

  free (line);
  return !_quit_request;
}

/* Execute a command line. */
bool execute_line (const GlobalData& globalData, char *_line) {
  int i = 0;
  COMMAND *command;
  char *word;
  char *line = strdup(_line);
  if (!line) {
    return false;
  }

  /* Isolate the command word. */
  i = 0;
  while (line[i] && whitespace (line[i]))
    ++i;
  word = line + i;

  while (line[i] && !whitespace (line[i]))
    ++i;

  bool args = line[i] != 0;
  if (args)
    line[i] = '\0';

  command = find_command (word);

  if(args)
    line[i++] = ' '; // reinsert space

  if (!command)
    {
      std::cerr << word << ": No such command" << std::endl;
      return false;
    }

  /* Get argument to command, if any. */
  while (whitespace (line[i]))
    ++i;

  word = line + i;

  /* Call the function. */
  bool rv = ((*(command->func)) (globalData, line, word));
  free(line);
  return rv;
}

/* Look up NAME as the name of a command, and return a pointer to that
   command.  Return a nullptr pointer if NAME isn't a command name. */
COMMAND *find_command (char *name){
  int i = 0;
  char *p = strchr(name,'=');
  if(p) return (&commands[0]); // set a parameter.
  for (i = 0; commands[i].name; ++i)
    if (strcmp (name, commands[i].name) == 0)
      return (&commands[i]);

  return (static_cast<COMMAND *>(nullptr));
}

/* Strip whitespace from the start and end of STRING.  Return a pointer
   into STRING. */
char * stripwhite (char *string){
  char *s, *t;

  for (s = string; whitespace (*s); ++s)
    ;

  if (*s == 0)
    return (s);

  t = s + strlen (s) - 1;
  while (t > s && whitespace (*t))
    --t;
  *++t = '\0';

  return s;
}

/* **************************************************************** */
/*                                                                  */
/*                  Interface to Readline Completion                */
/*                                                                  */
/* **************************************************************** */

char *command_generator (const char *, int);
char *params_generator (const char *, int);
//char **console_completion __P((const char *, int, int));
char **console_completion (const char *, int, int);

/* Tell the GNU Readline library how to complete.  We want to try to
   complete on command names if this is the first word in the line, or
   on filenames if not. */
void initializeConsole ()
{
  /* Allow conditional parsing of the ~/.inputrc file. */
  rl_readline_name = "LPZRobots_Console";

  /* Tell the completer that we want a crack first. */
  rl_attempted_completion_function = console_completion;

  read_history (".history");
}

// store the history
void closeConsole(){
  write_history(".history");
}

int getListLen(char **strings) {
  int i=0;
  if(!strings) return 0;
  while ( strings[i] ){
    ++i;
  }
  return i;
}

/* Attempt to complete on the contents of TEXT.  START and END
   bound the region of rl_line_buffer that contains the word to
   complete.  TEXT is the word to complete.  We can use the entire
   contents of rl_line_buffer in case we want to do some simple
   parsing.  Return the array of matches, or nullptr if there aren't any. */
char ** console_completion (const char *text, int start, int end) {

  char **matchesCmd = static_cast<char **>(nullptr);
  char **matchesParams = static_cast<char **>(nullptr);

  /* If this word is at the start of the line, then it is a command
     to complete or a parameter name.
     Otherwise it is the name of a file in the current
     directory.
     if __PLACEHOLDER_40__ is at the start then it is also a parameter.
  */
  try{
    if (start == 0){
      matchesCmd = rl_completion_matches (text, command_generator);
    }
    if(start==0 || (strncmp(rl_line_buffer,"set",3)== 0)){
      matchesParams = rl_completion_matches (text, params_generator);
    }
  }catch(...){}
  // merge them;
  int lCmd=getListLen(matchesCmd);
  int lPar=getListLen(matchesParams);
  if(lCmd+lPar > 0){
    char **matches = static_cast<char **>(malloc((lCmd+lPar+1)*sizeof(char*)));
    memcpy(matches,matchesCmd,sizeof(char*)*lCmd);
    memcpy(matches+lCmd,matchesParams,sizeof(char*)*lPar);
    matches[lCmd+lPar]=static_cast<char *>(nullptr);
    return matches;
  } else
    return static_cast<char **>(nullptr);


}

/* Generator function for command completion.  STATE lets us
   know whether to start from scratch; without any state
   (i.e. STATE == nullptr), then we start at the top of the list. */
char * command_generator (const char *text, int state) {
  static int list_index, len;
  const char *name;

  /* If this is a new word to complete, initialize now.  This
     includes saving the length of TEXT for efficiency, and
     initializing the index variable to 0. */
  if (!state)
    {
      list_index = 1;
      len = strlen (text);
    }

  /* Return the next name which partially matches from the
     command list. */
  while ( (name = commands[list_index].name) )
    {
      ++list_index;

      if (strncmp (name, text, len) == 0)
        return (dupstr(name));
    }

  /* If no names matched, then return nullptr. */
  return (static_cast<char*>(nullptr));
}

/* Generator function for parameter completion.  STATE lets us
   know whether to start from scratch; without any state
   (i.e. STATE == nullptr), then we start at the top of the list. */
char * params_generator (const char *text, int state) {
  static int len;
  static ParameterList::iterator list_it;

  /* If this is a new word to complete, initialize now.  This
     includes saving the length of TEXT for efficiency, and
     initializing the index variable to 0. */
  if (!state)
    {
      list_it = parameters.begin();
      len = strlen (text);
    }

  /* Return the next name which partially matches from the
     parameter list. */
  while ( list_it != parameters.end())
    {
      if (list_it->find(text, 0, len) == 0){
        char* name = dupstrpluseq(list_it->c_str());
        ++list_it;
        return name;
      }
      ++list_it;
    }

  /* If no names matched, then return nullptr. */
  return (static_cast<char*>(nullptr));
}


/* **************************************************************** */
/*                                                                  */
/*                       Console Commands                           */
/*                                                                  */
/* **************************************************************** */


bool com_list (const GlobalData& globalData, char* line, char* arg) {
  int i=1;
  std::cout << "Agents -------------(for store and load)\nID: Name" << std::endl;

  FOREACHC(OdeAgentList, globalData.agents,a){
    if((*a)->getRobot())
    std::cout << " " << std::setw(3) << i << ": " << (*a)->getRobot()->getName() << " (Controller and Robot)" << std::endl;
    std::cout << " " << std::setw(3) << (i*100+1) << ":  |- only Controller" << std::endl;
    std::cout << " " << std::setw(3) << (i*100+2) << ":  |- only Robot" << std::endl;
    ++i;
  }
  std::cout << "Configurables ------(for set, show, storecfg and loadcfg )\nID: Name" << std::endl;
  i=1;
  for(auto c = globalData.globalconfigurables.begin(); c != globalData.globalconfigurables.end(); ++c){
    std::cout << " " << std::setw(2) << i << ": " << (*c)->getName() << std::endl;
    ++i;
  }
  return true;
}

bool com_show (const GlobalData& globalData, char* line, char* arg) {
  if (arg && *arg){
    int id = atoi(arg);
    if(id>=1 && id <= static_cast<int>(globalData.globalconfigurables.size())){
      auto it = globalData.globalconfigurables.begin();
      std::advance(it, id-1);
      printConfig(*it);
      return true;
    }
  }
  printConfigs(globalData.globalconfigurables);

  return true;
}

bool com_set (const GlobalData& globalData, char* line, char* arg) {
  if(strstr(line,"set")!=line) arg=line; // if it is not invoked with set then it was param=val
  bool changed=false;
  if (valid_argument("set", arg)){
    /* Isolate the command word. */

    char * equalpos = strchr(arg,'=');
    if(equalpos) {
      *equalpos=' '; // replace by space for splitting
    }else{
      std::cout << "Syntax error! no '=' found, see help" << std::endl;
      return true;
    }
    vector<string> params = splitstring(string(arg));
    switch(params.size()){
    case 3:// ObjectID param=val
      {
        int id = atoi(params[0].c_str());
        if(id>=1 && id <= static_cast<int>(globalData.globalconfigurables.size())){
          const char* key = params[1].c_str();
          auto it = globalData.globalconfigurables.begin();
          std::advance(it, id-1);
          if ((*it)->setParam(key,atof(params[2].c_str()))){
            std::cout << " " << key << "=\t" << (*it)->getParam(key) 
                      << "\t" << (*it)->getName() << std::endl;
            changed = true;
          }
        }else std::cout << "Object with ID: " << id << " not found" << std::endl;
      }
      break;
    case 2: // param=val
      {
        double v=atof(params[1].c_str());
        const char* key = params[0].c_str();
         for(auto i = globalData.globalconfigurables.begin(); i != globalData.globalconfigurables.end(); ++i){
           if ((*i)->setParam(key,v)){
             std::cout << " " << key << "=\t" << (*i)->getParam(key) 
                       << "\t" << (*i)->getName() << std::endl;
             changed = true;
           }
         }
      }
      break;
    default: // something else
      std::cout << "Syntax Error! Expect 2 or 3 arguments: [ObjectID] param=val" << std::endl;
      std::cout << "Got " << params.size() << " params:";
      FOREACHC(vector<string>, params, p) { std::cout << *p << ", "; }
      std::cout << std::endl;
      break;
    }
    // if(changed){ // now done for every command
    //   *equalpos=__PLACEHOLDER_111__;
    //   FOREACH(OdeAgentList, globalData.agents, i){
    //     (*i)->writePlotComment(arg);
    //   }
    // }
  }
  return changed; // true;
}

bool com_store (const GlobalData& globalData, char* line, char* arg) {
  bool success=false;
  if (valid_argument("store", arg)){
    char* filename;
    short sub=0;
    filename = strchr(arg,' ');
    if(filename) { // we have 2 arguments
      *filename='\0';
      ++filename;
      int id = atoi(arg);
      if(id>=100) {
        sub=id%100;
        id=id/100;
      }
      if(id>=1 && id <= static_cast<int>(globalData.agents.size())){
        FILE* f = fopen(filename,"wb");
        if(f){
          switch(sub){
          case 0: // store agent
            if(globalData.agents[id-1]->store(f)){
              std::cout << "Agent stored" << std::endl;
              success = true;
            } else std::cout << "Error occured while storing agent" << std::endl;
            break;
          case 1: // store controller
            // Controller doesn't have store method
            std::cout << "Controller storage not implemented" << std::endl;
            break;
          case 2: // store robot
            if(globalData.agents[id-1]->getRobot()->store(f)){
              std::cout << "Robot stored" << std::endl;
              success = true;
            }else std::cout << "Error occured while storing robot" << std::endl;
            break;
          }
          fclose(f);
        }else std::cout << "Cannot open file " << filename << " for writing" << std::endl;
      } else std::cout << "Agent with ID: " << id << " not found" << std::endl;
    }else std::cout << "syntax error , see >help store" << std::endl;
  }
  return success;
}

bool com_load (const GlobalData& globalData, char* line, char* arg) {
  bool success=false;
  if (valid_argument("load", arg)){
    char* filename;
    short sub=0;
    filename = strchr(arg,' ');
    if(filename) { // we have 2 arguments
      *filename='\0';
      ++filename;
      int id = atoi(arg);
      if(id>=100) {
        sub=id%100;
        id=id/100;
      }
      if(id>=1 && id <= static_cast<int>(globalData.agents.size())){
        FILE* f = fopen(filename,"rb");
        if(f){
          switch(sub){
          case 0: // store agent
            if(globalData.agents[id-1]->restore(f)){
              std::cout << "Agent restored" << std::endl;
              success = true;
            }
            else std::cout << "Error occured while restoring agent" << std::endl;
            break;
          case 1: // store controller
            // Controller doesn't have restore method
            std::cout << "Controller restore not implemented" << std::endl;
            break;
          case 2: // store robot
            if(globalData.agents[id-1]->getRobot()->restore(f)){
              std::cout << "Robot restored" << std::endl;
              success = true;
            }
            else std::cout << "Error occured while restoring robot" << std::endl;
            break;
          }
          fclose(f);
        }else std::cout << "Cannot open file " << filename << " for reading" << std::endl;
      } else std::cout << "Agent with ID: " << id << " not found" << std::endl;
    }else std::cout << "syntax error , see >help load" << std::endl;
  }
  return success;
}

bool com_storecfg (const GlobalData& globalData, char* line, char* arg) {
  bool success=false;
  if (valid_argument("storecfg", arg)){
    char* filename;
    filename = strchr(arg,' ');
    if(filename) { // we have 2 arguments
      *filename='\0';
      ++filename;
      int id = atoi(arg);
      if(id>=1 && id <= static_cast<int>(globalData.globalconfigurables.size())){
        auto it = globalData.globalconfigurables.begin();
        std::advance(it, id-1);
        if((*it)->storeCfg(filename)){
          std::cout << "Configuration stored" << std::endl;
          success = true;
        }else
          std::cout << "Error occured while storing configuration" << std::endl;
      } else std::cout << "Configurable with ID: " << id << " not found" << std::endl;
    }else std::cout << "syntax error , see >help storecfg" << std::endl;
  }
  return success;
}

bool com_loadcfg (const GlobalData& globalData, char* line, char* arg) {
  bool success=false;
  if (valid_argument("loadcfg", arg)){
    char* filename;
    filename = strchr(arg,' ');
    if(filename) { // we have 2 arguments
      *filename='\0';
      ++filename;
      int id = atoi(arg);
      if(id>=1 && id <= static_cast<int>(globalData.globalconfigurables.size())){
        auto it = globalData.globalconfigurables.begin();
        std::advance(it, id-1);
        if((*it)->restoreCfg(filename)){
          std::cout << "Configuration restored" << std::endl;
          success = true;
        } else
          std::cout << "Error occured while restoring configuration" << std::endl;
      } else std::cout << "Configurable with ID: " << id << " not found" << std::endl;
    }else std::cout << "syntax error , see >help loadcfg" << std::endl;
  }
  return success;
}

bool com_contrs (const GlobalData& globalData, char* line, char* arg) {
  if (valid_argument("contours", arg)){
    char* filename;
    filename = arg;
    if(filename) { // we at least 1 argument
      FILE* f = fopen(filename,"wb");
      if(f){
        int i=0;
        FOREACHC(ObstacleList, globalData.obstacles, o) {
          AbstractGround* g = dynamic_cast<AbstractGround*>(*o);
          if(g){
            fprintf(f, "# Contour from Playground %i\n", i);
            g->printContours(f);
            ++i;
          }
        }
        std::cout << i << " playground contours saved to " << filename << std::endl;
        fclose(f);
      }else std::cout << "Cannot open file " << filename << " for writing" << std::endl;
    }else std::cout << "syntax error , see >help store" << std::endl;
  }
  return true;
}


bool com_quit (const GlobalData& globalData, char *, char *){
  _quit_request=true;
  return true;
}

/* Print out help for ARG, or for all of the commands if ARG is
   not present. */
bool com_help (const GlobalData& globalData, char* line, char* arg) {
  int i = 0;
  int printed = 0;

  for (i = 0; commands[i].name; ++i)
    {
      if (!*arg || (strcmp (arg, commands[i].name) == 0))
        {
          std::cout << " " << commands[i].name << "\t\t" << commands[i].doc << "." << std::endl;
          ++printed;
        }
    }

  if (!printed)
    {
      std::cout << "No commands match `" << arg << "'.  Possibilties are:" << std::endl;

      for (i = 0; commands[i].name; ++i)
        {
          /* Print in six columns. */
          if (printed == 6)
            {
              printed = 0;
              std::cout << std::endl;
            }

          std::cout << " " << commands[i].name << "\t";
          ++printed;
        }

      if (printed)
        std::cout << std::endl;
    }
  return true;
}



/* Return non-zero if ARG is a valid argument for CALLER,
   else print an error message and return zero. */
int
valid_argument ( const char *caller, const char *arg)
{
  if (!arg || !*arg)
    {
      std::cerr << caller << ": Argument required." << std::endl;
      return (0);
    }

  return (1);
}

}
