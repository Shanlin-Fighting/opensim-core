#ifndef OPENSIM_ACTUATOR_H_
#define OPENSIM_ACTUATOR_H_

// Actuator.h
// Author: Ajay Seth
/*
 * Copyright (c)  2010, Stanford University. All rights reserved. 
* Use of the OpenSim software in source form is permitted provided that the following
* conditions are met:
* 	1. The software is used only for non-commercial research and education. It may not
*     be used in relation to any commercial activity.
* 	2. The software is not distributed or redistributed.  Software distribution is allowed 
*     only through https://simtk.org/home/opensim.
* 	3. Use of the OpenSim software or derivatives must be acknowledged in all publications,
*      presentations, or documents describing work in which OpenSim or derivatives are used.
* 	4. Credits to developers may not be removed from executables
*     created from modifications of the source.
* 	5. Modifications of source code must retain the above copyright notice, this list of
*     conditions and the following disclaimer. 
* 
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
*  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
*  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
*  SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
*  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
*  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR BUSINESS INTERRUPTION) OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
*  WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <OpenSim/Simulation/osimSimulationDLL.h>
#include "Force.h"


#ifdef SWIG
	#ifdef OSIMSIMULATION_API
		#undef OSIMSIMULATION_API
		#define OSIMSIMULATION_API
	#endif
#endif

namespace OpenSim {

class Model;
class VisibleObject;
class Controller;
class Coordinate;

//=============================================================================
//                   ACTUATOR_ (vector of control values)
//=============================================================================
/**
 * Base class for an actuator (e.g., a torque motor, muscle, ...) that requires
 * external input (controls) to generate force.
 *
 * @author Ajay Seth
 */
class OSIMSIMULATION_API Actuator_ : public Force {
OpenSim_DECLARE_ABSTRACT_OBJECT(Actuator_, Force);
//=============================================================================
// NO PROPERTIES
//=============================================================================

//=============================================================================
// DATA
//=============================================================================
protected:
 
    // index in Controls Vector shared system cache entry
	int _controlIndex;

//=============================================================================
// METHODS
//=============================================================================
	//--------------------------------------------------------------------------
	// CONSTRUCTION
	//--------------------------------------------------------------------------
public:
	Actuator_();

    // default destructor, copy constructor, copy assignment

private:
	void setNull();

	//--------------------------------------------------------------------------
	// GET AND SET
	//--------------------------------------------------------------------------
protected:
	// ModelComponent Interface
	virtual void addToSystem(SimTK::MultibodySystem& system) const;

	// Update the geometry attached to the actuator. Use inertial frame.
	virtual void updateGeometry();

public:

	//Model building
	virtual int numControls() const = 0;

	/** Actuator default controls are zero */
	virtual const SimTK::Vector getDefaultControls() { return SimTK::Vector(numControls(), 0.0); } 
#ifndef SWIG
	// CONTROLS
	virtual const SimTK::VectorView_<double> getControls( const SimTK::State& s ) const;
#endif
	/** Convenience methods for getting, setting and adding to actuator controls from/into 
	    the model controls. These methods have no effect on the realization stage. */
	virtual void getControls(const SimTK::Vector& modelControls, SimTK::Vector& actuatorControls) const;
	/** set actuator controls subvector into the right slot in the system-wide model controls */
	virtual void setControls(const SimTK::Vector& actuatorControls, SimTK::Vector& modelControls) const;
	/** add actuator controls to the values already occupying the slot in the system-wide model controls */
	virtual void addInControls(const SimTK::Vector& actuatorControls, SimTK::Vector& modelControls) const;

	//--------------------------------------------------------------------------
	// COMPUTATIONS
	//--------------------------------------------------------------------------
	virtual double computeActuation( const SimTK::State& s) const = 0;
	virtual void computeEquilibrium(SimTK::State& s) const { }


//=============================================================================
};	// END of class Actuator_
//=============================================================================


//==============================================================================
//                       ACTUATOR (scalar control value)
//==============================================================================

/**
 * Derived class for an actuator (e.g., a torque motor, muscle, ...) that 
 * requires exactly one external input (control) to generate a scalar
 * value force, such as a torque/force magnitude or a tension.
 *
 * @author Ajay Seth
 */
class OSIMSIMULATION_API Actuator : public Actuator_ {
OpenSim_DECLARE_ABSTRACT_OBJECT(Actuator, Actuator_);
public:
//==============================================================================
// PROPERTIES
//==============================================================================
    /** @name Property declarations 
    These are the serializable properties associated with this class. **/
    /**@{**/
    /** Default is -Infinity (no limit). **/
	OpenSim_DECLARE_PROPERTY(min_control, double,
		"Minimum allowed value for control signal. Used primarily when solving "
        "for control values.");
    /** Default is Infinity (no limit). **/
	OpenSim_DECLARE_PROPERTY(max_control, double,
		"Maximum allowed value for control signal. Used primarily when solving "
        "for control values.");
    /**@}**/

//==============================================================================
// PUBLIC METHODS
//==============================================================================
	Actuator();

    // default destructor, copy constructor, copy assignment

	/** Convenience method to set controls given scalar (double) valued control */
	//virtual void setControl(const SimTK::State &s, double control) const;

	/** Convenience method to get control given scalar (double) valued control */
	virtual double getControl(const SimTK::State& s ) const;

	//Model building
	virtual int numControls() const {return 1;};

	// Accessing force, speed, and power of a scalar valued actuator
	virtual void setForce(const SimTK::State& s, double aForce) const; 
    virtual double getForce( const SimTK::State& s) const;
    virtual void setSpeed( const SimTK::State& s, double aspeed) const;
    virtual double getSpeed( const SimTK::State& s) const;
	virtual double getPower(const SimTK::State& s) const { return getForce(s)*getSpeed(s); }
	virtual double getStress(const SimTK::State& s) const;
	virtual double getOptimalForce() const;

	// manage bounds on Control
	void setMinControl(const double& aMinControl) 
    {   set_min_control(aMinControl); }
	double getMinControl() const { return get_min_control(); }
	void setMaxControl(const double& aMaxControl) 
    {   set_max_control(aMaxControl); }
	double getMaxControl() const { return get_max_control(); }

    ///--------------------------------------------------------------------------
    /// Overriding forces
    ///--------------------------------------------------------------------------
    /// The force normally produced by an Actuator can be overriden and 
    /// When the Actuator's force is overriden, the Actuator will by defualt
    /// produce a constant force which can be set with setOverrideForce().  
    /**
    * enable/disable an Actuator's override force
    *  
    * @param s    current state
    * @param flag true = override Actuator's output force
    *             false = use Actuator's computed forc (normal operation)
    */
    void overrideForce(SimTK::State& s, bool flag ) const;

    /**
    *  return Actuator's override status
    */
    bool isForceOverriden(const SimTK::State& s ) const;

    /**
    * set the force value  used when  the override is true 
    * 
    * @param s      current state
    * @param value  value of override force   
    */
    void setOverrideForce(SimTK::State& s, double value ) const;

    /**
    * return override force 
    */
    double getOverrideForce(const SimTK::State& s ) const;


protected:
	// ModelComponent Interface
	virtual void addToSystem(SimTK::MultibodySystem& system) const;

	double computeOverrideForce(const SimTK::State& s ) const;

	//Force reporting
	/** 
	 * Methods to query a Force for the value actually applied during simulation
	 * The names of the quantities (column labels) is returned by this first function
	 * getRecordLabels()
	 */
	OpenSim::Array<std::string> getRecordLabels() const {
		OpenSim::Array<std::string> labels("");
		labels.append(getName());
		return labels;
	}
	/**
	 * Given SimTK::State object extract all the values necessary to report forces, application location
	 * frame, etc. used in conjunction with getRecordLabels and should return same size Array
	 */
	OpenSim::Array<double> getRecordValues(const SimTK::State& state) const {
		OpenSim::Array<double> values(1);
		values.append(getForce(state));
		return values;
	}

private:
	void constructProperties();

//=============================================================================
};	// END of class Actuator
//=============================================================================

} // end of namespace OpenSim

#endif // OPENSIM_ACTUATOR_H_


