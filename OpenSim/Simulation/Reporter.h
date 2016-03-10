#ifndef OPENSIM_REPORTER_H_
#define OPENSIM_REPORTER_H_
/* -------------------------------------------------------------------------- *
 *                             OpenSim:  Reporter.h                              *
 * -------------------------------------------------------------------------- *
 * The OpenSim API is a toolkit for musculoskeletal modeling and simulation.  *
 * See http://opensim.stanford.edu and the NOTICE file for more information.  *
 * OpenSim is developed at Stanford University and supported by the US        *
 * National Institutes of Health (U54 GM072970, R24 HD065690) and by DARPA    *
 * through the Warrior Web program.                                           *
 *                                                                            *
 * Copyright (c) 2005-2016 Stanford University and the Authors                *
 * Author(s): Ajay Seth                                        *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0.         *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */
// INCLUDE
#include <OpenSim/Common/Component.h>

namespace OpenSim {

class TimeSeriesTable;

/**
 * This abstract class represents a Reporter that collects and reports Outputs
 * computed and generated by Components in a Model during a Simulation. The 
 * Inputs of a Reporter are the named Outputs of Components that compose a
 * Model.
 *
 * @author Ajay Seth
 */
class OSIMSIMULATION_API Reporter : public Component {
OpenSim_DECLARE_ABSTRACT_OBJECT(Reporter, ModelComponent);
public:
//==============================================================================
// PROPERTIES
//==============================================================================
    OpenSim_DECLARE_PROPERTY(is_disabled, bool,
        "Flag indicating whether the Reporter is disabled or not. Disabled means"
        " that the Reporter does not record in subsequent Report realizations.");

    OpenSim_DECLARE_PROPERTY(report_time_iterval, double,
        "The recording time interval (s). If interval < 0 or NaN defaults to"
        "every valid integration time step.");

//==============================================================================
// INPUTS
//==============================================================================
    /** Variable list of Inputs by name that will be recorded by the Reporter 
        and corresponds to a Model (Component) Output during a simulation. */
    OpenSim_DECLARE_INPUTS("output_names", SimTK::Report,
        " The Outputs listed by name that are to be recorded on each reporting "
        " event that occurs during a simulation.");

//=============================================================================
// PUBLIC METHODS
//=============================================================================

    /** Report values given the state and top-level Component (e.g. Model) */
    void report(const SimTK::State& s);

    /** Report values given the state and top-level Component (e.g. Model) */
    const TimeSeriesTable& getReport();


    /** Return if the Reporter is disabled or not. */
    bool isDisabled(const SimTK::State& s) const;
    /** %Set the Reporter as disabled (true) or not (false). */
    void setDisabled(SimTK::State& s, bool disabled) const;



protected:
    /** Default constructor sets up Reporter-level properties; can only be
    called from a derived class constructor. **/
    Reporter();

    /** Deserialization from XML, necessary so that derived classes can 
    (de)serialize. **/
    Reporter(SimTK::Xml::Element& node) : Super(node);
    {   setNull(); constructProperties(); }


    //--------------------------------------------------------------------------
    // Reporter interface.
    //--------------------------------------------------------------------------
    virtual void implementReport(const SimTK::State& s);

    //--------------------------------------------------------------------------
    // Component interface.
    //--------------------------------------------------------------------------
    /** Add reporter required resources to the underlying System */
    void extendAddToSystem(SimTK::MultibodySystem& system) const override;


private:
    void setNull();
    void constructProperties();

    /** The TimeSeriesTable that contains the report  */
    TimeSeriesTable _reportTable;

//=============================================================================
};  // END of class Reporter
//=============================================================================
//=============================================================================

} // end of namespace OpenSim

#endif // OPENSIM_REPORTER_H_


