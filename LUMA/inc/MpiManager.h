/*
 * --------------------------------------------------------------
 *
 * ------ Lattice Boltzmann @ The University of Manchester ------
 *
 * -------------------------- L-U-M-A ---------------------------
 *
 *  Copyright (C) The University of Manchester 2017
 *  E-mail contact: info@luma.manchester.ac.uk
 *
 * This software is for academic use only and not available for
 * further distribution commericially or otherwise without written consent.
 *
 */

#ifndef MPIMAN_H
#define MPIMAN_H

#include "stdafx.h"
#include "HDFstruct.h"
class GridObj;
class GridManager;


// Define the loop expressions required to inspect the overlap regions of a grid for ease of coding
#define range_i_left	i = 0; i < GridUtils::downToLimit((int)pow(2, g->level + 1), N_lim); i++	///< For loop definition for left halo
#define range_j_down	j = 0; j < GridUtils::downToLimit((int)pow(2, g->level + 1), M_lim); j++	///< For loop definition for bottom halo
#define range_k_front	k = 0; k < GridUtils::downToLimit((int)pow(2, g->level + 1), K_lim); k++	///< For loop definition for front halo
#define range_i_right	i = GridUtils::upToZero(N_lim - (int)pow(2, g->level + 1)); i < N_lim; i++	///< For loop definition for right halo
#define range_j_up		j = GridUtils::upToZero(M_lim - (int)pow(2, g->level + 1)); j < M_lim; j++	///< For loop definition for top halo
#define range_k_back	k = GridUtils::upToZero(K_lim - (int)pow(2, g->level + 1)); k < K_lim; k++	///< For loop definition for back halo

/// \brief	MPI Manager class.
///
///			Class to manage all MPI apsects of the code.
class MpiManager
{
	///	Class to hold smart decomposition data
	class SDData
	{
	public:

		SDData() {};
		~SDData() {};
		// Copy constructor
		SDData(SDData& other)
			: XSol(other.XSol), YSol(other.YSol), ZSol(other.ZSol),
			theta(other.theta), thetaNew(other.thetaNew), delta(other.delta)
		{ };

		// Solution vectors
		std::vector<double> XSol;	///< Solution vector for X variables in SD problem.
		std::vector<double> YSol;	///< Solution vector for Y variables in SD problem.
		std::vector<double> ZSol;	///< Solution vector for Z variables in SD problem.

		// Iteration vectors
		std::vector<double> theta;		///< Complete vector of variable block edges.
		std::vector<double> delta;		///< Perturbation vector.
		std::vector<double> thetaNew;	///< Perturbed vector of block edges.
	};

	/// class to hold imbalance information
	class LoadImbalanceData
	{
	public:
		LoadImbalanceData()
		{
			heaviestBlock.resize(3);
		};
		~LoadImbalanceData() {};

		// Copy constructor
		LoadImbalanceData(LoadImbalanceData& other)
			: loadImbalance(other.loadImbalance), uniImbalance(other.uniImbalance),
			heaviestBlock(other.heaviestBlock), heaviestOps(other.heaviestOps)
		{ };

		double loadImbalance;		///< Imbalance assocaited with smart decomposition.
		double uniImbalance;		///< Imbalance assocaited with uniform decomposition.
		size_t heaviestOps;			///< Number of operations on heaviest rank.
		std::vector<unsigned int> heaviestBlock;	///< MPI indices of the heaviest block.
	};


private :
	MpiManager();			///< Private constructor
	~MpiManager();			///< Private destructor
	static MpiManager* me;	///< Pointer to self

public :

	/************** Member Data **************/

	// MPI world data (all public)
	MPI_Comm world_comm;	///< Global MPI communicator

	/// \brief	Cartesian unit vectors pointing to each neighbour in Cartesian topology.
	///
	///			Define 3D such that first 8 mimic the 2D ones. Opposites are 
	///			simply the next or previous column in the array. MSVC 2013 does
	///			not support initialiser lists tagged onto the constructor although
	///			it is valid C++ so I have had to make it static even though it goes
	///			against the idea of the singleton design.
	static const int neighbour_vectors[3][26];
	int dimensions[3];							///< Size of MPI Cartesian topology
	int neighbour_rank[L_MPI_DIRS];				///< Neighbour rank number for each direction in Cartesian topology
	int neighbour_coords[L_DIMS][L_MPI_DIRS];	///< Coordinates in MPI topology of neighbour ranks

	// Sizes of each of the MPI domains
	/// Number of sites in X direction for each custom rank.
	std::vector<int> cRankSizeX;
	/// Number of sites in Y direction for each custom rank.
	std::vector<int> cRankSizeY;
	/// Number of sites in Z direction for each custom rank.
	std::vector<int> cRankSizeZ;
	
	/// Communicators for sub-grid / region combinations
#if (L_NUM_LEVELS > 0)
	MPI_Comm subGrid_comm[L_NUM_LEVELS * L_NUM_REGIONS];	
#else
	MPI_Comm subGrid_comm[1];	// Default to size = 1
#endif
	
	// Commonly used properties of the rank / topology
	int my_rank;				///< Rank number
	int num_ranks;				///< Total number of ranks in MPI Cartesian topology
	int rank_coords[L_DIMS];	///< Coordinates in MPI Cartesian topology


	/// \brief	Absolute positions of edges of the core region represented on this rank.
	///
	///			Excludes outer overlapping layer (recv layer). 
	///			Rows are x,y,z start and end pairs and columns are rank number.
	///			Access the rows using the eCartMinMax enumeration.
	std::vector< std::vector<double> > rank_core_edge;


	/// \struct layer_edges
	/// \brief	Structure containing absolute positions of the edges of halos.
	///
	///			Sender (inner) and receiver (outer) parts of halo are located 
	///			using the convention [left_min left_max right_min right_max] 
	///			for X and similar for Y and Z. Access using the enumerator 
	///			eEdgeMinMax.
	struct layer_edges {
		double X[4];	///< X limits
		double Y[4];	///< Y limits
		double Z[4];	///< Z limits
	};
	layer_edges sender_layer_pos;	///< Structure containing sender layer edge positions.
	layer_edges recv_layer_pos;		///< Structure containing receiver layer edge positions.
	

	// Buffer data
	std::vector< std::vector<double>> f_buffer_send;	///< Array of resizeable outgoing buffers used for data transfer
	std::vector< std::vector<double>> f_buffer_recv;	///< Array of resizeable incoming buffers used for data transfer
	MPI_Status recv_stat;					///< Status structure for Receive return information
	MPI_Request send_requests[L_MPI_DIRS];	///< Array of request structures for handles to posted ISends
	MPI_Status send_stat[L_MPI_DIRS];		///< Array of statuses for each ISend

	/// \struct buffer_struct
	/// \brief	Structure storing buffers sizes in each direction for particular grid.
	struct buffer_struct
	{
		int size[L_MPI_DIRS];	///< Buffer sizes for each direction
		int level;				///< Grid level
		int region;				///< Region number

		buffer_struct(int l, int r) 
			: level(l), region(r){};
	};
	std::vector<buffer_struct> buffer_send_info;	///< Vectors of buffer_info structures holding sender layer size info.
	std::vector<buffer_struct> buffer_recv_info;	///< Vectors of buffer_info structures holding receiver layer size info.

	/// Logfile handle
	std::ofstream* logout;



	/************** Member Methods **************/

	// Singleton design
	static MpiManager* getInstance();	// Get the pointer to the singleton instance (create it if necessary)
	static void destroyInstance();

	// Initialisation
	void mpi_init();												// Initialisation of MpiManager & Cartesian topology
	void mpi_gridbuild(GridManager* const grid_man);				// Do domain decomposition to build local grid dimensions
	int mpi_buildCommunicators(GridManager* const grid_man);		// Create a new communicator for each sub-grid and region combo
	void mpi_updateLoadInfo(GridManager* const grid_man);			// Method to compute the number of active cells on the rank and pass to master
	void mpi_uniformDecompose(int *numCells);						// Method to perform uniform decomposition into MPI blocks
	LoadImbalanceData mpi_smartDecompose(double dh,
		std::vector<int> combo = std::vector<int>(0));				// Method to perform load-balanced decomposition into MPI blocks
	void mpi_reportOnDecomposition(double dh);						// Method to provide a report on decomposition options
	void mpi_SDReconstructSolution(SDData& solutionData, std::vector<int>& numCores);
	void mpi_SDComputeImbalance(LoadImbalanceData& load, SDData& solutionData, std::vector<int>& numCores);
	bool mpi_SDCheckDelta(SDData& solutionData, double dh, std::vector<int>& numCores);
	void mpi_SDCommunicateSolution(SDData& solutionData, double imbalance, double dh);

	// Buffer methods
	void mpi_buffer_pack(int dir, GridObj* const g);		// Pack the buffer ready for data transfer on the supplied grid in specified direction
	void mpi_buffer_unpack(int dir, GridObj* const g);		// Unpack the buffer back to the grid given
	void mpi_buffer_size();									// Set buffer size information for grids in hierarchy given and 
															// set pointer to hierarchy for subsequent access
	void mpi_buffer_size_send( GridObj* const g );			// Routine to find the size of the sending buffer on supplied grid
	void mpi_buffer_size_recv( GridObj* const g );			// Routine to find the size of the receiving buffer on supplied grid

	// IO
	void mpi_writeout_buf(std::string filename, int dir);		// Write out the buffers of direction dir to file

	// Comms
	void mpi_communicate( int level, int regnum );		// Wrapper routine for communication between grids of given level/region
	int mpi_getOpposite(int direction);					// Version of GridUtils::getOpposite for MPI_directions rather than lattice directions

};

#endif