/*
* --------------------------------------------------------------
*
* ------ Lattice Boltzmann @ The University of Manchester ------
*
* -------------------------- L-U-M-A ---------------------------
*
*  Copyright (C) 2015, 2016
*  E-mail contact: info@luma.manchester.ac.uk
*
* This software is for academic use only and not available for
* distribution without written consent.
*
*/

#include "../inc/stdafx.h"
#include "../inc/IBInfo.h"


// Default constructor
supportCommSupportSideClass::supportCommSupportSideClass() {

	// Default values
	rank = 0;
	bodyID = 0;
}

/// \brief Custom constructor for support side communication class.
///	\param	rank to send to		index of body.
///	\param	marker				index of marker.
///	\param	support				index of support.
supportCommSupportSideClass::supportCommSupportSideClass(int rankID, int body, std::vector<int> &position) {

	// Default values
	rank = rankID;
	bodyID = body;
	supportIdx.push_back(position[eXDirection]);
	supportIdx.push_back(position[eYDirection]);
	supportIdx.push_back(position[eZDirection]);
}


// Default constructor
supportCommMarkerSideClass::supportCommMarkerSideClass() {

	// Default values
	bodyID = 0;
	markerID = 0;
	supportID = 0;
	rank = 0;
}

/// \brief Custom constructor for support receiver communication class.
///	\param	body				index of body.
///	\param	marker				index of marker.
///	\param	support				index of support.
supportCommMarkerSideClass::supportCommMarkerSideClass(int body, int marker, int support, int rankID) {

	// Default values
	bodyID = body;
	markerID = marker;
	supportID = support;
	rank = rankID;
}


// Default constructor
epsCalcMarkerClass::epsCalcMarkerClass() {

	// Default values
	bodyID = 0;
	local_area = 0.0;
	dilation = 0.0;
}


/// \brief Custom constructor for epsilon calculation class.
///	\param	ib				id of current body.
///	\param	positionIn		position of IB marker.
///	\param	areaIn			local area of IB marker.
///	\param	dilationIn		dilation parameter for IB marker.
///	\param	supp_position	positions of all support points for that IB marker.
///	\param	deltavalIn		delta value for all support points for that IB marker.
epsCalcMarkerClass::epsCalcMarkerClass(int bodyIDIn, std::vector<double> positionIn, double areaIn, double dilationIn, std::vector<std::vector<double>> supp_position, std::vector<double> deltavalIn) {

	// Assign values
	bodyID = bodyIDIn;
	local_area = areaIn;
	dilation = dilationIn;

	// Resize vectors and assign
	position.resize(positionIn.size());
	deltaval.resize(deltavalIn.size());
	position = positionIn;
	deltaval = deltavalIn;

	// Assign position of support points
	for (int i = 0; i < deltaval.size(); i++) {
		x.push_back(supp_position[i][eXDirection]);
		y.push_back(supp_position[i][eYDirection]);

#if (L_DIMS == 3)
		z.push_back(supp_position[i][eZDirection]);
#endif
	}
}
