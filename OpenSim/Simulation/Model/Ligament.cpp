/* -------------------------------------------------------------------------- *
 *                           OpenSim:  Ligament.cpp                           *
 * -------------------------------------------------------------------------- *
 * The OpenSim API is a toolkit for musculoskeletal modeling and simulation.  *
 * See http://opensim.stanford.edu and the NOTICE file for more information.  *
 * OpenSim is developed at Stanford University and supported by the US        *
 * National Institutes of Health (U54 GM072970, R24 HD065690) and by DARPA    *
 * through the Warrior Web program.                                           *
 *                                                                            *
 * Copyright (c) 2005-2012 Stanford University and the Authors                *
 * Author(s): Peter Loan                                                      *
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

//=============================================================================
// INCLUDES
//=============================================================================
#include "Ligament.h"
#include "GeometryPath.h"
#include "PointForceDirection.h"
#include <OpenSim/Common/SimmSpline.h>

//=============================================================================
// STATICS
//=============================================================================
using namespace std;
using namespace OpenSim;
using SimTK::Vec3;

//=============================================================================
// CONSTRUCTOR(S) AND DESTRUCTOR
//=============================================================================
//_____________________________________________________________________________
// Default constructor.
Ligament::Ligament()
{
	constructProperties();
}


//_____________________________________________________________________________
/*
 * Construct and initialize the properties for the ligament.
 * 
 * You should give each property a meaningful name and an informative comment.
 * The name you give each property is the tag that will be used in the XML
 * file. The comment will appear before the property in the XML file.
 * In addition, the comments are used for tool tips in the OpenSim GUI.
 *
 * All properties are added to the property set. Once added, they can be
 * read in and written to file.
 */
void Ligament::constructProperties()
{
	setAuthors("Peter Loan");
	constructProperty_GeometryPath(GeometryPath());
	constructProperty_resting_length(0.0);
	constructProperty_pcsa_force(0.0);

	int forceLengthCurvePoints = 13;
	double forceLengthCurveX[] = {-5.00000000,  0.99800000,  0.99900000,  1.00000000,  1.10000000,  1.20000000,  1.30000000,  1.40000000,  1.50000000,  1.60000000,  1.60100000,  1.60200000,  5.00000000};
	double forceLengthCurveY[] = {0.00000000,  0.00000000,  0.00000000,  0.00000000,  0.03500000,  0.12000000,  0.26000000,  0.55000000,  1.17000000,  2.00000000,  2.00000000,  2.00000000,  2.00000000};
	SimmSpline forceLengthCurve
       (forceLengthCurvePoints, forceLengthCurveX, forceLengthCurveY);

	constructProperty_force_length_curve(forceLengthCurve);
}

//_____________________________________________________________________________
/**
 * Perform some setup functions that happen after the
 * ligament has been deserialized or copied.
 *
 * @param aModel model containing this ligament.
 */
void Ligament::connectToModel(Model& aModel)
{
	GeometryPath& path = upd_GeometryPath();
	const double& restingLength = get_resting_length();

	// Specify underlying ModelComponents prior to calling 
    // Super::connectToModel() to automatically propagate connectToModel()
    // to subcomponents. Subsequent addToSystem() will also be automatically
	// propagated to subcomponents.
    // TODO: this is awkward; subcomponent API needs to be revisited (sherm)
	includeAsSubComponent(&path);

    //TODO: can't call this at start of override; this is an API bug.
	Super::connectToModel(aModel);

	// _model will be NULL when objects are being registered.
	if (_model == NULL)
		return;

	// Resting length must be greater than 0.0.
	assert(restingLength > 0.0);

	path.setOwner(this);
}

//_____________________________________________________________________________
/**
 * allocate and initialize the SimTK state for this ligament.
 */
 void Ligament::addToSystem(SimTK::MultibodySystem& system) const
{
	Super::addToSystem(system);
}


void Ligament::initStateFromProperties( SimTK::State& s) const
{
	Super::initStateFromProperties(s);
}


//=============================================================================
// GET AND SET
//=============================================================================
//-----------------------------------------------------------------------------
// LENGTH
//-----------------------------------------------------------------------------
//_____________________________________________________________________________
/**
 * Get the length of the ligament. This is a convenience function that passes
 * the request on to the ligament path.
 *
 * @return Current length of the ligament path.
 */
double Ligament::getLength(const SimTK::State& s) const
{
	return getGeometryPath().getLength(s);
}

//_____________________________________________________________________________
/**
 * Set the resting length.
 *
 * @param aRestingLength The resting length of the ligament.
 * @return Whether the resting length was successfully changed.
 */
bool Ligament::setRestingLength(double aRestingLength)
{
	set_resting_length(aRestingLength);
	return true;
}

//_____________________________________________________________________________
/**
 * Set the maximum isometric force.
 *
 * @param aMaxIsometricForce The maximum isometric force of the ligament.
 * @return Whether the maximum isometric force was successfully changed.
 */
bool Ligament::setMaxIsometricForce(double aMaxIsometricForce)
{
	set_pcsa_force(aMaxIsometricForce);
	return true;
}

//_____________________________________________________________________________
/**
 * Set the force-length curve.
 *
 * @param aForceLengthCurve Pointer to a force-length curve (Function).
 * @return Whether the force-length curve was successfully changed.
 */
bool Ligament::setForceLengthCurve(const Function& aForceLengthCurve)
{
	set_force_length_curve(aForceLengthCurve);
	return true;
}

//_____________________________________________________________________________
/**
 * Set this ligament to have linear stiffness. This method will in turn
 * set the ForceLengthCurve to a straight line, and the maxIsoForce
 * to the stiffness value (for unit strain).
 *
 * @param aStiffness the linear stiffness value for this ligament
 * @param aRestLength the rest length of the spring
 */
void Ligament::setLinearStiffness(double aStiffness, double aRestLength)
{
    // the ligament force uses the normalized stretch (strain), therefore to
    // mimic a linear spring behaviour the force-length curve must incorporate
    // the rest length.
    // the resulting force term is:  f = K * (l-l0)

    LinearFunction linearFunc(aRestLength, 0);
    set_force_length_curve(linearFunc);
    set_resting_length(aRestLength);
    set_pcsa_force(aStiffness);
}

//=============================================================================
// SCALING
//=============================================================================
//_____________________________________________________________________________
/**
 * Perform computations that need to happen before the ligament is scaled.
 * For this object, that entails calculating and storing the
 * length in the current body position.
 *
 * @param aScaleSet XYZ scale factors for the bodies.
 */
void Ligament::preScale(const SimTK::State& s, const ScaleSet& aScaleSet)
{
	updGeometryPath().preScale(s, aScaleSet);
}

//_____________________________________________________________________________
/**
 * Scale the ligament.
 *
 * @param aScaleSet XYZ scale factors for the bodies
 * @return Whether or not the ligament was scaled successfully
 */
void Ligament::scale(const SimTK::State& s, const ScaleSet& aScaleSet)
{
	updGeometryPath().scale(s, aScaleSet);
}

//_____________________________________________________________________________
/**
 * Perform computations that need to happen after the ligament is scaled.
 * For this object, that entails comparing the length before and after scaling,
 * and scaling the resting length a proportional amount.
 *
 * @param aScaleSet XYZ scale factors for the bodies.
 */
void Ligament::postScale(const SimTK::State& s, const ScaleSet& aScaleSet)
{
	GeometryPath& path          = updGeometryPath();
	double&       restingLength = upd_resting_length();

	path.postScale(s, aScaleSet);

	if (path.getPreScaleLength(s) > 0.0)
	{
		double scaleFactor = path.getLength(s) / path.getPreScaleLength(s);

		// Scale resting length by the same amount as the change in
		// total ligament length (in the current body position).
		restingLength *= scaleFactor;

		path.setPreScaleLength(s, 0.0);
	}
}

//=============================================================================
// COMPUTATION
//=============================================================================
/**
 * Compute the moment-arm of this muscle about a coordinate.
 */
double Ligament::computeMomentArm(const SimTK::State& s, Coordinate& aCoord) const
{
	return getGeometryPath().computeMomentArm(s, aCoord);
}



void Ligament::computeForce(const SimTK::State& s, 
							  SimTK::Vector_<SimTK::SpatialVec>& bodyForces, 
							  SimTK::Vector& generalizedForces) const
{
	const GeometryPath& path = getGeometryPath();
	const double& restingLength = get_resting_length();
	const double& pcsaForce = get_pcsa_force();

	if (path.getLength(s) <= restingLength)
		return;

	double strain = (path.getLength(s) - restingLength) / restingLength;
	double force = getForceLengthCurve().calcValue(SimTK::Vector(1, strain)) 
                                                                   * pcsaForce;

	OpenSim::Array<PointForceDirection*> PFDs;
	path.getPointForceDirections(s, &PFDs);

	for (int i=0; i < PFDs.getSize(); i++) {
		applyForceToPoint(s, PFDs[i]->body(), PFDs[i]->point(), 
                          force*PFDs[i]->direction(), bodyForces);
	}
}

double Ligament::getTension(const SimTK::State& s) const {

    const GeometryPath& path = getGeometryPath();
    const double& restingLength = get_resting_length();
    const double& pcsaForce = get_pcsa_force();

    if (path.getLength(s) <= restingLength)
        return 0;

    double strain = (path.getLength(s) - restingLength) / restingLength;
    return getForceLengthCurve().calcValue(SimTK::Vector(1, strain))
                                                                   * pcsaForce;
}

//_____________________________________________________________________________
/**
 * Get the visible object used to represent the Ligament.
 */
VisibleObject* Ligament::getDisplayer() const
{ 
	return getGeometryPath().getDisplayer(); 
}

//_____________________________________________________________________________
/**
 * Update the visible object used to represent the Ligament.
 */
void Ligament::updateDisplayer(const SimTK::State& s)
{
	updGeometryPath().updateDisplayer(s);
}
