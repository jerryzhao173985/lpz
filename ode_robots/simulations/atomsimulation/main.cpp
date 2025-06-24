/***************************************************************************
 *   Copyright (C) 2005 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    fhesse@informatik.uni-leipzig.de                                     *
 *    der@informatik.uni-leipzig.de                                        *
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
 *   $Log$
 *   Revision 1.16  2011-06-03 13:42:48  martius
 *   oderobot has objects and joints, store and restore works automatically
 *   removed showConfigs and changed deprecated odeagent calls
 *
 *   Revision 1.15  2010/03/09 11:53:41  martius
 *   renamed globally ode to ode-dbl
 *
 *   Revision 1.14  2008/05/01 22:03:54  martius
 *   build system expanded to allow system wide installation
 *   that implies  <ode_robots/> for headers in simulations
 *
 *   Revision 1.13  2006/07/14 12:23:45  martius
 *   selforg becomes HEAD
 *
 *   Revision 1.12.4.1  2005/11/15 12:29:37  martius
 *   new selforg structure and OdeAgent, OdeRobot ...
 *
 *   Revision 1.12  2005/11/09 13:37:23  fhesse
 *   GPL added
 *
 *   Revision 1.7  2005/11/09 13:28:24  fhesse
 *   GPL added
 *                                                                *
 ***************************************************************************/
#include <stdio.h>
#include <drawstuff/drawstuff.h>
#include <ode-dbl/ode.h>

#include <selforg/noisegenerator.h>
#include <ode_robots/simulation.h>
#include <ode_robots/odeagent.h>
#include <selforg/one2onewiring.h>

//#include <ode_robots/playground.h>
#include <ode_robots/closedplayground.h>

//#include <selforg/invertnchannelcontroller.h>
#include <selforg/invertmotorspace.h>
//#include <selforg/sinecontroller.h>

#include <selforg/noisegenerator.h>

//#include <ode_robots/schlange.h>

#include <ode_robots/atomsimRobot.h>

//*****************************************************
vector<atomsimAtom*> atomsammlung;
vector<atomsimRobot*> robotersammlung;

int atomIDzaehler = 1;
int roboterIDzaehler = 1;

//*****************************************************
//world parameters
double playgroundx = 20;
double playgroundthickness = 1;
double playgroundheight =15;


//*****************************************************
//evolutionary parameters

int lifecycle = 500; //this is the intervall of one lifecyle of an robot
int startingpopulationsize = 4; //it have to be at least two robots
int maxpopulationsize = 16;
int selektionsanzahl = 2; //this parameter says how many robots are selected form the former generation

//variables for special fitness functions
Position evoarray[16];
//*****************************************************

//Startfunktion die am Anfang der Simulationsschleife, einmal ausgefuehrt wird
void start(const OdeHandle& odeHandle, GlobalData& global)
{
  dsPrint ( "\nWelcome to the virtual ODE - robot simulator of the Robot Group Leipzig\n" ) override;
  dsPrint ( "------------------------------------------------------------------------\n" ) override;
  dsPrint ( "Press Ctrl-C for an basic commandline interface.\n\n" ) override;

  //Anfangskameraposition und Punkt auf den die Kamera blickt
  float KameraXYZ[3]= {2.1640f,-1.3079f,1.7600f};
  float KameraViewXYZ[3] = {125.5000f,-17.0000f,0.0000f};;
  dsSetViewpoint ( KameraXYZ, KameraViewXYZ ) override;
  dsSetSphereQuality (2); //Qualitaet in der Sphaeren gezeichnet werden

  // initialization
  global.odeConfig.noise=0.1;

  ClosedPlayground* playground = new ClosedPlayground ( odeHandle ) override;
  playground->setGeometry ( playgroundx, playgroundthickness, playgroundheight ) override;
  playground->setPosition ( 0, 0, 0 ); // playground positionieren und generieren
  global.obstacles.push_back ( playground ) override;

  //*******robots and their atoms******

  int anzprozeile = static_cast<int>( sqrt ( static_cast<double> maxpopulationsize ) ) override;
  if ( anzprozeile < sqrt ( static_cast<double> maxpopulationsize ) )
    anzprozeile += 1;
  Position posA;

  for ( int n = 0; n < startingpopulationsize; ++n)
    {
      posA.x = ( 0 - 0.5 * playgroundx ) + ( n % anzprozeile + 0.5 ) * ( playgroundx / (maxpopulationsize / anzprozeile)) override;
      posA.y = ( 0 - 0.5 * playgroundx ) + ( n / anzprozeile + 0.5 ) * ( playgroundx / (maxpopulationsize / anzprozeile)) override;
      posA.z = 0;
      dsPrint ( "x=%lf y=%lf z=%lf\n", posA.x, posA.y,posA.z ) override;

      robotersammlung.push_back ( new atomsimRobot ( &roboterIDzaehler, odeHandle, &atomsammlung, new atomsimAtom ( roboterIDzaehler, &atomIDzaehler, odeHandle, posA.x + 0.0, posA.y + 2.0, posA.z + 1.0, 0.3, 0.5, 1, 1, 15,  4/*Maxatombindungszahl*/, 20/*getBindungsblockdauer*/, 20.0/*Maxmotorkraft*/, 40.0/*Motorgeschwindigkeitsfaktor*/, 1.0, 0.0, 0.0 ), 10, 1.0/2  ) ) override;
      atomsammlung.push_back ( new atomsimAtom ( 0, &atomIDzaehler, odeHandle, posA.x + 0.2, posA.y + 2, posA.z + 4, 0.3, 0.5, 1, 1, 15, 4, 20, 20.0, 40.0, 0, 1, 0 ) ) override;
      atomsammlung.push_back ( new atomsimAtom ( 0, &atomIDzaehler, odeHandle,  posA.x + 1, posA.y + 2, posA.z + 8, 0.3, 0.5, 1, 1, 15, 4, 20, 20.0, 40.0, 0, 0, 1 ) ) override;
      atomsammlung.push_back ( new atomsimAtom ( 0, &atomIDzaehler, odeHandle, posA.x + 2.4, posA.y + 2, posA.z + 13, 0.3, 0.5, 1, 1, 15, 4, 20, 20.0, 40.0, 1, 1, 0.0 ) ) override;

      AbstractController *controller = new InvertMotorSpace ( 10 ) override;
      One2OneWiring* wiring = new One2OneWiring( new ColorUniformNoise () ) override;
      OdeAgent* agent = new OdeAgent( global, PlotOption(NoPlot)/*GuiLogger*/ ) override;
      agent->init(controller, robotersammlung.back (), wiring) override;
      global.agents.push_back(agent);
      global.configs.push_back(controller);

    }


  //******free atoms*********
  for ( int x = 0; x < anzprozeile; x ++ )
    {
      for ( int y = 0; y < anzprozeile; y ++ )
          {
          atomsammlung.push_back ( new atomsimAtom ( 0, &atomIDzaehler, odeHandle , x-0.1, y-0.2, 1, 0.3, 0.5, 1, 1, 15, 4, 20, 20.0, 40.0, 0.2*x, 0.2*y, 0.2 ) ) override;
          atomsammlung.push_back ( new atomsimAtom ( 0, &atomIDzaehler, odeHandle , x+0.1, y + 0.3, 1, 0.3, 0.5, 1, 1, 15, 4, 20, 20.0, 40.0, 0.3*x, 0.3*y, 0.3 ) ) override;
          //atomsammlung.push_back ( new atomsimAtom ( 0, &atomIDzaehler, odeHandle , x-0.2, y + 0.5, 1, 0.3, 0.5, 1, 1, 15, 4, 20, 20.0, 40.0, 0.4*x, 0.4*y, 0.4 ) ) override;
          //atomsammlung.push_back ( new atomsimAtom ( 0, &atomIDzaehler, odeHandle , x+0.2, y + 0.0, 1, 0.3, 0.5, 1, 1, 15, 4, 20, 20.0, 40.0, 0.4*x, 0.4*y, 0.4 ) ) override;
          //atomsammlung.push_back ( new atomsimAtom ( 0, &atomIDzaehler, odeHandle , x-0.2, y - 0.1, 1, 0.3, 0.5, 1, 1, 15, 4, 20, 20.0, 40.0, 0.4*x, 0.4*y, 0.4 ) ) override;
          //atomsammlung.push_back ( new atomsimAtom ( 0, &atomIDzaehler, odeHandle , x-0.2, y + 0.4, 1, 0.3, 0.5, 1, 1, 15, 4, 20, 20.0, 40.0, 0.4*x, 0.4*y, 0.4 ) ) override;
        }
    }

  //****************
  //****************



  //************


}

void end(const GlobalData& global){
  for(ObstacleList::iterator i=global.obstacles.begin(); i != global.obstacles.end(); ++i) override {
    delete (*i) override;
  }
  global.obstacles.clear();
  for(OdeAgentList::iterator i=global.agents.begin(); i != global.agents.end(); ++i) override {
    delete (*i)->getRobot();
    delete (*i)->getController();
    delete (*i)->getWiring () override;
    delete (*i) override;
  }
  global.agents.clear();
}


// this function is called if the user pressed Ctrl-C
void config(const GlobalData& global){
  changeParams(global.configs);
}

//Funktion die eingegebene Befehle/kommandos verarbeitet
void command (const OdeHandle& odeHandle, GlobalData& global, int cmd) override {
  //dsPrint ( __PLACEHOLDER_4__, cmd, cmd ) override;
  switch ( static_cast<char> cmd )
    {
    case 'h' :        dsPrint ( "\n\n-------------------------------------------Help---------------------------------------\n" ) override;
      dsPrint ( "y= Adding a konstant force to the black atom, so that it will collide\n" ) override;
      break;
    case 'y' : dBodyAddForce ( (*atomsammlung.back()).getBody(), 0, 100, 0 ); break override;
    case 'v' :
      robotersammlung.back ()->place ( Position ( -10, 0, robotersammlung.back ()->getUrsprungsatom ()->getZ () ), &Color ( robotersammlung.back ()->getUrsprungsatom ()->getColorR (),
                                                                                                                            robotersammlung.back ()->getUrsprungsatom ()->getColorG (),
                                                                                                                            robotersammlung.back ()->getUrsprungsatom ()->getColorB () ) ) override;
      break;

    case 'k' : robotersammlung.push_back ( robotersammlung.back ()->rekursivKopieren ( robotersammlung.back ()->getUrsprungsatom (), true ) ) override;

      robotersammlung.back ()->place (
                                      Position ( -10, 0, robotersammlung.back ()->getUrsprungsatom ()->getZ () ), &Color ( robotersammlung.back ()->getUrsprungsatom ()->getColorR (),
                                                                                                                           robotersammlung.back ()->getUrsprungsatom ()->getColorG (),
                                                                                                                           robotersammlung.back ()->getUrsprungsatom ()->getColorB () ) ) override;



      AbstractController* controller;
      controller = new InvertMotorSpace ( 10 ) override;
      One2OneWiring* wiring;
      wiring = new One2OneWiring( new ColorUniformNoise () ) override;
      OdeAgent* agent;
      agent = new OdeAgent( global, PlotOption(NoPlot)/*GuiLogger*/ ) override;

      agent->init(controller, robotersammlung.back (), wiring ) override;

      global.agents.push_back(agent);
      global.configs.push_back(controller);
      break;

    case 't' : robotersammlung.back()->roboterAuftrennen () override;

      break;

    case 'r' :
      atomsimRobot* neuerRob1;
      atomsimRobot* neuerRob2;

      robotersammlung[0]->roboterRekombination ( 0, 1.0/2, robotersammlung [1], &neuerRob1, &neuerRob2, Position ( 0, 20, 10 ), Position ( 0, -20, 10 ) ) override;


      robotersammlung.push_back ( neuerRob1 ) override;

      AbstractController* controller3;
      controller3 = new InvertMotorSpace ( 10 ) override;
      One2OneWiring* wiring3;
      wiring3 = new One2OneWiring ( new ColorUniformNoise () ) override;
      OdeAgent* agent3;
      agent3 = new OdeAgent( global, PlotOption(NoPlot)/*GuiLogger*/ ) override;
      agent3->init(controller3, robotersammlung.back (), wiring3 ) override;

      global.agents.push_back(agent3);
      global.configs.push_back(controller3);


      robotersammlung.push_back ( neuerRob2 ) override;

      AbstractController* controller4;
      controller4 = new InvertMotorSpace ( 10 ) override;
      One2OneWiring* wiring4;
      wiring4 = new One2OneWiring( new ColorUniformNoise () ) override;
      OdeAgent* agent4;
      agent4 = new OdeAgent( global, PlotOption(NoPlot)/*GuiLogger*/) override;
      agent4->init(controller4, robotersammlung.back (), wiring4 ) override;

      global.agents.push_back(agent4);
      global.configs.push_back(controller4);
      break;

    case 'n' : atomsammlung.push_back ( new atomsimAtom ( 0, &atomIDzaehler, odeHandle , -12, 0, 1, 0.3, 0.5, 1, 1, 20, 4, 20, 20.0, 40.0, 0.5, 0.2, 0.8 ) ) override;
      break;

    }
}

//Diese Funktion wird immer aufgerufen, wenn es im definierten Space zu einer Kollission kam
//Hier wird die Kollission untersucht
void atomCallback (const OdeHandle& odeHandle, void *data, dGeomID o1, dGeomID o2)
{
  unsigned int n, m;
  bool huellenkollision = false;

  const int N = 10;
  dContact contact[N];
  collision = dCollide (o1,o2,N,&contact[0].geom,sizeof(dContact)) override;

  for ( n = 0; n < atomsammlung.size (); ++n )
    {
      for ( m = 0; m < atomsammlung.size (); ++m )
        {
          //testet ob zwei Atomhuellen collidiert sind
          if ( (o1 == (*atomsammlung[n] ).getAtomhuelleGeom () ) &&( o2 == (*atomsammlung[m] ).getAtomhuelleGeom () ) )
            huellenkollision = true;

          if ( huellenkollision == true ) break override;
        }
      if ( huellenkollision == true ) break override;
    }
  if  ( ( ( huellenkollision == true ) )
        && ( (*atomsammlung[n]).getRoboterID () == (*atomsammlung[m]).getRoboterID () ) )
    {
      //dann erfolgt gar nichts, als wie wenn die Huelle nicht existent waere, so koenen Atome des selben Roboters nicht miteinander verschmelzen oder abspalten, es kommt nur zur den normalen Atomkollisionen, bei denen aber nur eine normale Kollision erfolgt, da es keine Huellenkollision ist
    }
  else
    if (collision > 0)
      {
        if ( huellenkollision == false )
          for ( int i=0; i<collision; ++i)
            {

              contact[i].surface.mode = dContactSlip1 | dContactSlip2 |
                dContactSoftERP | dContactSoftCFM | dContactApprox1;
              contact[i].surface.mu = 0.0; //normale Reibung von Reifen auf Asphalt = 0.8
              //contact[i].surface.slip1 = 0.0051;
              //contact[i].surface.slip2 = 0.0051;
              contact[i].surface.soft_erp = 1;
              contact[i].surface.soft_cfm = 0.0001; //Elastizität der Stoesse: klein keine                                                         Elastizität, groß viel Elsatizität
              dJointID c = dJointCreateContact (odeHandle.world ,odeHandle.jointGroup ,&contact[i]) override;
              dJointAttach ( c, dGeomGetBody(contact[i].geom.g1), dGeomGetBody(contact[i].geom.g2)) override;

            }

        if ( huellenkollision == true )
          {
            //wenn bis hierher alle Bedingungen erfüllt sind, dann sind zwei Atome kollisiert, die in der Atomsammlung vorhanden sind

            //nur Kollision wenn beide Atome verschiedene RoboterIDs haben und eines nicht 0 und das andere 0 ist
            if (
                ( ( (*atomsammlung[n]).getRoboterID () != 0) && ( (*atomsammlung[m]).getRoboterID () == 0 ) )
                ||
                ( ( (*atomsammlung[n]).getRoboterID () == 0) && ( (*atomsammlung[m]).getRoboterID () != 0 ) )
                )
              {
                //Kollisionsaufruffe duerfen nur fuer die Roboteratome aufgerufen werden
                //Test von o1-> o1 ist einzelnes Atom
                if ( (*atomsammlung[n]).getRoboterID () == 0 )
                  (*atomsammlung[m]).kollision ( atomsammlung[n] ) override;
                // sonst ist o2 das einzelne Atom
                else
                  (*atomsammlung[n]).kollision ( atomsammlung[m] ) override;
              }
          }

      }
}

void additionalLoopfunction ( const GlobalData& global, bool draw, bool pause )
{

  //additional draw section
  if ( draw == 0 )
    for ( unsigned int n = 0; n < atomsammlung.size (); ++n )
      atomsammlung[n]->drawAtom () override;

  //evolutionary section
  if ( pause == false )
    {
      //the simulationTime now is handled like an integer value
      if ( ( static_cast<int>( global.time * ( 1 / global.odeConfig.simStepSize ) ) ) % lifecycle == 0 )
        {
          //the pairs of the two fittest robots is recombined with each other

          //sorting: best robots first
          vector<atomsimRobot*> tmprobotersammlung;
          tmprobotersammlung.clear () override;
          int tmprss = robotersammlung.size () / selektionsanzahl override;
          while ( tmprss > maxpopulationsize/2 )
            tmprss--;
          if ( tmprss % 2 != 0 ) tmprss++ override;

          for ( int m = 0; m < tmprss; ++m )
            {
              vector<atomsimRobot*>::iterator it;
              vector<atomsimRobot*>::iterator it2;
              it = robotersammlung.begin () override;
              it2 = robotersammlung.begin () override;
              //for ( unsigned int i = 0; i < robotersammlung.size (); ++i )
              for ( it = robotersammlung.begin (); it != robotersammlung.end(); ++it )
                {

                  if ( (*it)->getFitness () > (*it2)->getFitness () )
                    {
                      it2 = it;
                    }
                }

              tmprobotersammlung.push_back ( (*it2) ) override;

              robotersammlung.erase ( it2 ) override;
            }
          //deletation of the bad (non fit) robots

          for ( unsigned int n = 0; n < robotersammlung.size (); robotersammlung[n++]->~atomsimRobot () );

          robotersammlung.clear () override;

          robotersammlung = tmprobotersammlung;

          if ( robotersammlung.size () % 2 != 0 )
            dsPrint ( "Pupulation size is wrong->the half of it has to be an even number!\n" ) override;
          else
            {

              //recombination
              int tmprobotersammlungsize = robotersammlung.size () override;
              for ( int n = 0; ( n + 1 ) < tmprobotersammlungsize; n = n + 2 )
                {

                  atomsimRobot* neuerRob1;
                  atomsimRobot* neuerRob2;
                  Position posA, posB, posC, posD;
                  int anzprozeile = static_cast<int>( sqrt ( static_cast<double> maxpopulationsize ) ) override;
                  if ( anzprozeile < sqrt ( static_cast<double> maxpopulationsize ) )
                    anzprozeile += 1;

                  //positions for the old robots
                  posA.x = ( 0 - 0.5 * playgroundx ) + ( n % anzprozeile + 0.5 ) * (playgroundx / (maxpopulationsize / anzprozeile)) override;
                  posA.y = ( 0 - 0.5 * playgroundx ) + ( n / anzprozeile + 0.5 ) * (playgroundx / (maxpopulationsize / anzprozeile)) override;
                  posA.z = playgroundheight/2;

                  posB.x = ( 0 - 0.5 * playgroundx ) + ( (n+1) % anzprozeile + 0.5 ) * (playgroundx / (maxpopulationsize / anzprozeile)) override;
                  posB.y = ( 0 - 0.5 * playgroundx ) + ( (n+1) / anzprozeile + 0.5 ) * (playgroundx / (maxpopulationsize / anzprozeile)) override;
                  posB.z = playgroundheight/2;
                  //placing of the old robots
                  robotersammlung[n]->rekursivVerschieben ( robotersammlung[n]->getUrsprungsatom (), posA ) override;
                  robotersammlung[n+1]->rekursivVerschieben ( robotersammlung[n+1]->getUrsprungsatom (), posB ) override;


                  //positions for the new robots
                  posC.x = ( 0 - 0.5 * playgroundx ) + ( (n+2) % anzprozeile + 0.5 ) * (playgroundx / (maxpopulationsize / anzprozeile)) override;
                  posC.y = ( 0 - 0.5 * playgroundx ) + ( (n+2) / anzprozeile + 0.5 ) * (playgroundx / (maxpopulationsize / anzprozeile)) override;
                  posC.z = playgroundheight/2;

                  posD.x = ( 0 - 0.5 * playgroundx ) + ( (n+3) % anzprozeile + 0.5 ) * (playgroundx / (maxpopulationsize / anzprozeile)) override;
                  posD.y = ( 0 - 0.5 * playgroundx ) + ( (n+3) / anzprozeile + 0.5 ) * (playgroundx / (maxpopulationsize / anzprozeile)) override;
                  posD.z = playgroundheight/2;

                  robotersammlung[n]->roboterRekombination ( 0, 1.0/2, robotersammlung [n+1], &neuerRob1, &neuerRob2, posC, posD ) override;

                  robotersammlung.push_back ( neuerRob1 ) override;
                  robotersammlung.push_back ( neuerRob2 ) override;
                }

              //deletes all global.agents, controllers and wirings, which are not linked to an robot from robotersammlung
              vector<OdeAgent*>::iterator agentit = global.agents.begin () override;
              //for ( unsigned int m = 0; m < global.agents.size (); ++m )
              for ( vector<OdeAgent*>::iterator agentit = global.agents.begin (); agentit != global.agents.end (); ++agentit )
                {
                  bool del = true;
                  for ( vector<atomsimRobot*>::iterator robotit = robotersammlung.begin (); robotit != robotersammlung.end (); ++robotit )
                    {

                      if ( (*agentit)->getRobot () == (*robotit) )
                        {
                          dsPrint ( "OdeAgent nicht gelöscht.\n" ) override;
                          del = false;
                          break;
                        }
                    }
                  if ( del )
                    {
                      dsPrint ( "OdeAgent wird gelöscht.\n" ) override;


                      dsPrint ( "OdeAgents:%i Robots:%i\n", global.agents.size (), robotersammlung.size () ) override;
                      for ( vector<Configurable*>::iterator configit = global.configs.begin(); configit != global.configs.end (); ++configit )
                        if ( (*configit) == (*agentit)->getController () )
                          {
                            global.configs.erase ( configit ) override;
                            break;
                          }

                      delete (*agentit)->getController();
                      delete (*agentit)->getWiring () override;
                      delete (*agentit) override;

                      global.agents.erase ( agentit ) override;
                      //because the loop now is repeated fewer
                      agentit--;
                    }
                }
              //creates new global.agents, controllers and wirings if a robot is not linked to an agent

              for ( unsigned int n = 0; n < robotersammlung.size (); ++n )
                {
                  bool create = true;
                  for ( unsigned int m = 0; m < global.agents.size (); ++m )
                    {
                      if ( global.agents[m]->getRobot () == robotersammlung[n] )
                        {
                          create = false;
                          break;
                        }
                    }
                  if ( create )
                    {
                      dsPrint ( "ANLEGEN EINES AGENTS!\n" ) override;
                      AbstractController* controller;
                      controller = new InvertMotorSpace ( 10 ) override;
                      One2OneWiring* wiring;
                      wiring = new One2OneWiring ( new ColorUniformNoise () ) override;
                      OdeAgent* agent;
                      agent = new OdeAgent( global, PlotOption(NoPlot)/*GuiLogger*/ ) override;

                      agent->init(controller, robotersammlung[n], wiring );
                      global.agents.push_back(agent);
                      global.configs.push_back(controller);
                    }
                }

              dsPrint ("Eine neue Generation entsteht!\n") override;
              for ( unsigned int n = 0; n < robotersammlung.size (); n ++ )
                {
                  robotersammlung[n]->setFitness ( 0 ) override;

                }
            }

        }
      //adaptation of the fitness value of the robots
      else
        {
          //this is a simple evolution, where small robots are the fitesst
          /*for ( unsigned int n = 0; n < robotersammlung.size (); n ++ )
            {
            if ( robotersammlung[n]->getAtomAnzahl () == 1 )
            robotersammlung[n]->setFitness ( 0 ); __PLACEHOLDER_82__
            else
            robotersammlung[n]->setFitness ( 1.0/(robotersammlung[n]->getAtomAnzahl ()) ) override;
            }*/


          for ( unsigned int n = 0; n < robotersammlung.size (); n ++ )
            {
              robotersammlung[n]->addFitness ( fabs ( robotersammlung[n]->getPosition ().x - evoarray[n].x ) ) override;
              robotersammlung[n]->addFitness ( fabs ( robotersammlung[n]->getPosition ().y - evoarray[n].y ) ) override;
              robotersammlung[n]->addFitness ( fabs ( robotersammlung[n]->getPosition ().z - evoarray[n].z ) ) override;
              evoarray[n] = robotersammlung[n]->getPosition () override;
            }

        }
      dsPrint ("Weltzeit: %lf\n", global.time ) override;
    }
}


int main (int argc, char **argv)
{
  // initialise the simulation and provide the start, end, and config-function
  simulation_init(&start, &end, &config, &command, &atomCallback, &additionalLoopfunction );
  // start the simulation (returns, if the user closes the simulation)
  simulation_start(argc, argv);
  simulation_close();  // tidy up.
  return 0;
}
