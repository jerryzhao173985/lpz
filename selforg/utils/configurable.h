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
#ifndef CONFIGURABLE_H
#define CONFIGURABLE_H

#include <cstdlib>
#include <iostream>
#include <optional>

#include "backcaller.h"
#include "stl_adds.h"
#include <limits>
#include <list>
#include <map>
#include <string>
#include <utility>

/**
 * Abstract class for configurable objects
 * (mainly controllers, but also obstacles and the like)
 * 
 * Provides a uniform way to handle configurable parameters 
 * with a key/value interface with caching, bounds checking, and more
 */
class Configurable : public BackCaller {
public:
  using paramkey = std::string;
  using paramdescr = std::string;
  // params of type double
  using paramval = double;
  using paramlist = std::list<std::pair<paramkey, paramval>>;
  using parammap = std::map<paramkey, paramval*>;

  // params of type bool
  using parambool = bool;
  using paramboollist = std::list<std::pair<paramkey, parambool>>;
  using paramboolmap = std::map<paramkey, parambool*>;

  // params of type int
  using paramint = int;
  using paramintlist = std::list<std::pair<paramkey, paramint>>;
  using paramintmap = std::map<paramkey, paramint*>;

  using paramdescrmap = std::map<paramkey, paramdescr>;

  // stuff for bounds
  using paramvalBounds = std::pair<paramval, paramval>;
  using paramvalBoundsMap = std::map<paramkey, paramvalBounds>;
  static constexpr paramval valDefMaxBound = 10.0;
  static constexpr paramval valDefMinBound = -10.0;

  using paramintBounds = std::pair<paramint, paramint>;
  using paramintBoundsMap = std::map<paramkey, paramintBounds>;
  static constexpr paramint intDefMinBound = -10;
  static constexpr paramint intDefMaxBound = 10;

  using paramvalpair = std::pair<paramkey, paramval*>;
  using paramboolpair = std::pair<paramkey, parambool*>;
  using paramintpair = std::pair<paramkey, paramint*>;

  using configurableList = std::vector<Configurable*>;

  /// nice predicate function for finding by ID
  struct matchId {
    using argument_type = Configurable*;
    using result_type = bool;

    explicit matchId(int id_) : id(id_) {}
    int id = 0;
    bool operator()(const Configurable* c) {
      return c->id == id;
    }
  };

  Configurable()
    : id(rand())
    , parent(nullptr) {}
  /// intialise with name and revision (use __PLACEHOLDER_2__)
  Configurable(const std::string& name, const std::string& revision)
    : id(rand())
    , name(name)
    , revision(revision)
    , parent(nullptr) {}

  //   Configurable(const Configurable& copy)
  //     :id(copy.id), name(copy.name), revision(revision),
  //      mapOfValues(mapOfValues), mapOfBoolean(mapOfBoolean), mapOfInteger(mapOfInteger)
  //   {
  //   }

  ~Configurable() override {}

  /**
     Is called when a parameter was changes via setParam(). Note that
     it is not called of parameters of childs are changed, then there notifyOnChange() method
     is called. The key and of the changed parameter
     (use getParam() to retrieve its actual value).
     Overload this function when special actions have to be taken on parameter changes.
  */
  virtual void notifyOnChange(const paramkey& key) {}

  /**
   This is the new style for adding configurable parameters. Just call this function
   for each parameter and you are done.
   If you need to do some special treatment for setting (or getting) of the parameter
   you can handle this by overloading getParam and setParam
   */
  virtual void addParameter(const paramkey& key,
                            paramval* val,
                            paramval minBound,
                            paramval maxBound,
                            const paramdescr& descr = paramdescr()) {
    mapOfValues[key] = val;
    if (minBound > *val)
      minBound = (*val) > 0 ? 0 : (*val) * 2;
    if (maxBound < *val)
      maxBound = (*val) > 0 ? (*val) * 2 : 0;
    if (!descr.empty())
      mapOfDescr[key] = descr;
    mapOfValBounds[key] = paramvalBounds(minBound, maxBound);
  }

  ///  See addParameter(const paramkey& key, paramval* val, paramval minBound, paramval maxBound,
  ///  const paramdescr& descr)
  virtual void addParameter(const paramkey& key,
                            paramval* val,
                            const paramdescr& descr = paramdescr()) {
    addParameter(key, val, valDefMinBound, valDefMaxBound, descr);
  }

  /**
   See addParameter(const paramkey& key, paramval* val) but for bool values
   */
  virtual void addParameter(const paramkey& key,
                            parambool* val,
                            const paramdescr& descr = paramdescr()) {
    mapOfBoolean[key] = val;
    if (!descr.empty())
      mapOfDescr[key] = descr;
  }

  /**
   See addParameter(const paramkey& key, paramval* val) but for int values
   */
  virtual void addParameter(const paramkey& key,
                            paramint* val,
                            paramint minBound,
                            paramint maxBound,
                            const paramdescr& descr = paramdescr()) {
    mapOfInteger[key] = val;
    if (minBound > *val)
      minBound = (*val) > 0 ? 0 : (*val) * 2;
    if (maxBound < *val)
      maxBound = (*val) > 0 ? (*val) * 2 : 0;
    if (!descr.empty())
      mapOfDescr[key] = descr;
    mapOfIntBounds[key] = paramintBounds(minBound, maxBound);
  }

  virtual void addParameter(const paramkey& key,
                            paramint* val,
                            const paramdescr& descr = paramdescr()) {
    addParameter(key, val, intDefMinBound, intDefMaxBound, descr);
  }

  /**
   This function is only provided for convenience. It does the same as addParameter but set the
   variable to the default value
   */
  virtual void addParameterDef(const paramkey& key,
                               paramval* val,
                               paramval def,
                               paramval minBound,
                               paramval maxBound,
                               const paramdescr& descr = paramdescr()) {
    *val = def;
    addParameter(key, val, minBound, maxBound, descr);
  }

  virtual void addParameterDef(const paramkey& key,
                               paramval* val,
                               paramval def,
                               const paramdescr& descr = paramdescr()) {
    addParameterDef(key, val, def, valDefMinBound, valDefMaxBound, descr);
  }

  /// See addParameterDef(const paramkey&, paramval*, paramval)
  virtual void addParameterDef(const paramkey& key,
                               parambool* val,
                               parambool def,
                               const paramdescr& descr = paramdescr()) {
    *val = def;
    addParameter(key, val, descr);
  }

  /// See addParameterDef(const paramkey&, paramval*, paramval)
  virtual void addParameterDef(const paramkey& key,
                               paramint* val,
                               paramint def,
                               paramint minBound,
                               paramint maxBound,
                               const paramdescr& descr = paramdescr()) {
    *val = def;
    addParameter(key, val, minBound, maxBound, descr);
  }

  virtual void addParameterDef(const paramkey& key,
                               paramint* val,
                               paramint def,
                               const paramdescr& descr = paramdescr()) {
    addParameterDef(key, val, def, intDefMinBound, intDefMaxBound, descr);
  }

  /// sets a description for the given parameter
  virtual void setParamDescr(const paramkey& key,
                             const paramdescr& descr,
                             bool traverseChildren = true);

  /// return the id of the configurable objects, which is created by random on initialisation
  int getId() const noexcept {
    return id;
  }

  /// return the name of the object
  virtual paramkey getName() const noexcept {
    return name;
  }

  /// returns the revision of the object
  virtual paramkey getRevision() const noexcept {
    return revision;
  }

  /**
   *  Sets the name of the configurable.
   * @param name the name to set
   * @param callSetNameOfInspectable if true and if this instance is also inspectable,
   * call automatically setNameOfInspectable(name).
   */
  virtual void setName(const paramkey& name, bool callSetNameOfInspectable = true);

  /// sets the revision Hint: {  return __PLACEHOLDER_3__; }
  virtual void setRevision(const paramkey& revision) {
    this->revision = revision;
  }

  /** returns the value of the requested parameter
   or 0 (+ error message to stderr) if unknown.
   */
  virtual paramval getParam(const paramkey& key, bool traverseChildren = true) const;

  /** C++17: returns the value of the requested parameter using std::optional
   * @return optional containing the value if found, empty optional otherwise
   */
  virtual std::optional<paramval> getParamOpt(const paramkey& key,
                                              bool traverseChildren = true) const;

  /**
   * Returns if the requested parameter is part of the configurable or their children
   * @param key to search for
   * @return true if key found, otherwise false
   */
  virtual bool hasParam(const paramkey& key, bool traverseChildren = true) const;

  /** sets the value of the given parameter
   or does nothing if unknown.
   */
  virtual bool setParam(const paramkey& key, paramval val, bool traverseChildren = true);

  /**
   * Sets the bounds (minBound and maxBound) of the given parameter.
   * Not useful for parambool.
   * If minBound=maxBound, it is threated as that no bound is given.
   */
  virtual void setParamBounds(const paramkey& key,
                              paramval minBound,
                              paramval maxBound,
                              bool traverseChildren = true);

  virtual void setParamBounds(const paramkey& key,
                              paramint minBound,
                              paramint maxBound,
                              bool traverseChildren = true);

  virtual void setParamBounds(const paramkey& key,
                              paramvalBounds bounds,
                              bool traverseChildren = true);

  virtual void setParamBounds(const paramkey& key,
                              paramintBounds bounds,
                              bool traverseChildren = true);

  /** The list of all parameters with there value as allocated lists.
      Note that these are only parameters that are managed manually (with setParam, getParam)
      @see getAllParamNames()
   @return list of key-value pairs
   */
  virtual paramlist getParamList() const {
    return paramlist(); // return an empty list
  }

  /// returns all names that are configureable
  virtual std::list<paramkey> getAllParamNames(bool traverseChildren = true) const;

  virtual parammap getParamValMap() const {
    return mapOfValues;
  }

  virtual paramintmap getParamIntMap() const {
    return mapOfInteger;
  }

  virtual paramboolmap getParamBoolMap() const {
    return mapOfBoolean;
  }

  /// returns the description for the given parameter
  virtual paramdescr getParamDescr(const paramkey& key, bool traverseChildren = true) const;

  virtual paramvalBounds getParamvalBounds(const paramkey& key, bool traverseChildren = true) const;

  virtual paramintBounds getParamintBounds(const paramkey& key, bool traverseChildren = true) const;

  virtual bool hasParamDescr(const paramkey& key, bool traverseChildren = true) const;

  virtual bool hasParamvalBounds(const paramkey& key, bool traverseChildren = true) const;

  virtual bool hasParamintBounds(const paramkey& key, bool traverseChildren = true) const;

  /** stores the key values paires into the file : filenamestem.cfg
   including the comments given in the list
   */
  virtual bool storeCfg(const char* filenamestem,
                        const std::list<std::string>& comments = std::list<std::string>());
  /** restores the key values paires from the file : filenamestem.cfg */
  virtual bool restoreCfg(const char* filenamestem);
  /// prints the keys, values and descriptions to the file. Each line is prefixed
  void print(FILE* f, const char* prefix, int columns = 90, bool traverseChildren = true) const;

  /// parses the configuration from the given file
  bool parse(FILE* f, const char* prefix = nullptr, bool traverseChildren = true);

  /**
   * Adds a configurable as a child object.
   * @param conf the instance to add
   */
  virtual void addConfigurable(Configurable* conf);

  /**
   * Removes a configurable as a child object.
   * @param conf the instance to remove
   */
  virtual void removeConfigurable(Configurable* conf);

  /**
   * Returns the list containing all configurable children.
   */
  virtual const configurableList& getConfigurables() const;

  /**
   * Indicates that the configurable itself or the configurable children
   * attached to this configurable have changed.
   * This method must be called manually so that the Configurator GUI can react
   * at the changes.
   * This is done through the callbackable interface
   * (using CallbackableType CALLBACK_CONFIGURABLE_CHANGED).
   */
  virtual void configurableChanged();

  static const CallbackableType CALLBACK_CONFIGURABLE_CHANGED = 11;

protected:
  /// copies the internal params of the given configurable
  void copyParameters(const Configurable&, bool traverseChildren = true);

  // internal function to print only description in multiline fasion
  void printdescr(FILE* f, const char* prefix, const paramkey& key, int columns, int indent) const;

private:
  int id;
  paramkey name;
  paramkey revision;

  parammap mapOfValues;
  paramboolmap mapOfBoolean;
  paramintmap mapOfInteger;
  paramdescrmap mapOfDescr;

  paramvalBoundsMap mapOfValBounds;
  paramintBoundsMap mapOfIntBounds;

  void initParamBounds(const paramkey& key);

  configurableList ListOfConfigurableChildren;
  Configurable* parent;
};

#endif // __CONFIGURABLE_H
