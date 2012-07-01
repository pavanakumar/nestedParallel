#ifndef __NESTED_PARALLEL_H__

#define __NESTED_PARALLEL_H__

#include<unistd.h>
#include<mpi.h>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<string>
#include<map>
#include<list>
#include<vector>
#include "dot_colors.h"

#define MAX_HOSTNAME_SIZE 256

class nestedParallel {
  public:
    nestedParallel( int *nargs , char ***args );  // Default constructior
    ~nestedParallel();  // Default constructior
    void log(); // writes log file 
    std::vector<int> &getDistributedMemoryRanks(); 
    MPI_Comm &getMyDistributedMemoryMPIComm();
    MPI_Comm &getMySharedMemoryMPIComm();
    const char *getMyDOTColor();
    bool isDistributedNode();
    int getMyHostID();
    int getMyRank();
    int getMyRankInSharedMemoryComm();
  private:
    /// Variables
    int _mpi_comm_size , _my_rank , _my_rank_shmem , _my_rank_distmem ;
    char _hostname[MAX_HOSTNAME_SIZE], *_buffer;
    bool _is_dist_node;
    /// Maps of mpi rank to host name
    std::map< int , std::string > _rank_hostname_map;
    std::map< std::string , int > _hostname_hostid_map;
    /// Maps of hostname to list of shared memeory rank
    std::map< std::string , std::vector<int> > _hostname_ranks_map;
    /// Communicator for all shmem cores in a given node
    std::map< int , MPI_Comm > _intera_node_mpicomm_map;
    std::map< int , MPI_Group > _intera_node_mpigroup_map;
    std::map< int , std::vector<int> > _node_shmem_ranks_map;
    /// The communicator for all distributed nodes
    MPI_Comm  _inter_node_mpicomm;
    MPI_Group _inter_node_mpigroup;
    /// The rank of mpi process responsible
    /// for doing distributed job
    std::vector<int> _node_ranks;
    std::string _log;

    // Member functions
    void createHostnameMap(); // Create the std::map of rank vs hostname
    void createSharedMemoryComm(); // Creates the shared memory MPI communicator
    void createDistributedMemoryComm(); // Creates the distributed memory MPI communicator
};

#endif

