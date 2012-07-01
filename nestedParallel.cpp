#include "nestedParallel.h"

nestedParallel::nestedParallel( int *nargs , char ***args ) {
  MPI_Init( nargs , args );
  MPI_Comm_size( MPI_COMM_WORLD , &_mpi_comm_size );
  MPI_Comm_rank( MPI_COMM_WORLD , &_my_rank );
  gethostname( _hostname , MAX_HOSTNAME_SIZE - 1 );
  createHostnameMap();
  createSharedMemoryComm();
  createDistributedMemoryComm();
  MPI_Barrier( MPI_COMM_WORLD );
}

void nestedParallel::createHostnameMap(){
  _buffer = new char[ MAX_HOSTNAME_SIZE * _mpi_comm_size ];
  MPI_Allgather( _hostname , MAX_HOSTNAME_SIZE , MPI_CHAR , 
                 _buffer , MAX_HOSTNAME_SIZE , MPI_CHAR , 
                 MPI_COMM_WORLD );
  // Create the rank hostname map
  for( int i = 0 ; i < _mpi_comm_size ; ++i ) {
    _rank_hostname_map[i] = &_buffer[ i * MAX_HOSTNAME_SIZE ];
    _hostname_ranks_map[ _rank_hostname_map[i] ].push_back(i);
  }
  for( int i = 0 ; i < _mpi_comm_size ; ++i ){   
    std::vector<int>::iterator v_beg = _hostname_ranks_map[ _rank_hostname_map[i] ].begin();
    std::vector<int>::iterator v_end = _hostname_ranks_map[ _rank_hostname_map[i] ].end();
    std::sort( v_beg , v_end ); 
  }
  free( _buffer );
}

void nestedParallel::createSharedMemoryComm(){
  MPI_Group old_group;
  int host_id = 0 , local_grp_sz;
  for( std::map<std::string , std::vector<int> >::iterator it = _hostname_ranks_map.begin();
          it != _hostname_ranks_map.end() ; ++it ){
    _hostname_hostid_map[ it->first ] = host_id;
    // The access operator [] allocates a key by 
    // default to the map using default constructor 
    // for the element of map
    MPI_Comm_group( MPI_COMM_WORLD , &old_group );
    MPI_Group_incl( old_group , it->second.size() , 
                    &it->second[0] , &(_intera_node_mpigroup_map[host_id]) );
    MPI_Comm_create( MPI_COMM_WORLD , _intera_node_mpigroup_map[host_id] , 
                     &(_intera_node_mpicomm_map[host_id]) );
    MPI_Comm_size( _intera_node_mpicomm_map[host_id] , &local_grp_sz );
    MPI_Group_size( _intera_node_mpigroup_map[host_id] , &local_grp_sz );
    host_id++;
  }
  MPI_Comm_rank( _intera_node_mpicomm_map[ getMyHostID() ] , &_my_rank_shmem );
}

void nestedParallel::createDistributedMemoryComm(){
  MPI_Group old_group;
  int dist_size;
  for( std::map<std::string , std::vector<int> >::iterator it = _hostname_ranks_map.begin();
          it != _hostname_ranks_map.end() ; ++it )
    _node_ranks.push_back( it->second[0] );
  MPI_Comm_group( MPI_COMM_WORLD , &old_group );
  MPI_Group_incl( old_group , _node_ranks.size() , &_node_ranks[0] , &(_inter_node_mpigroup) );
  MPI_Comm_create( MPI_COMM_WORLD , _inter_node_mpigroup , &(_inter_node_mpicomm) );
}

std::vector<int> &nestedParallel::getDistributedMemoryRanks() {
  return _node_ranks; 
} 
 
MPI_Comm &nestedParallel::getMySharedMemoryMPIComm() {
  return _intera_node_mpicomm_map[ getMyHostID() ]; 
}

MPI_Comm &nestedParallel::getMyDistributedMemoryMPIComm() {
  return _inter_node_mpicomm; 
}
const char *nestedParallel::getMyDOTColor() {
  return _dot_colors_svg[ _hostname_hostid_map[_hostname] ] ;    
}

int nestedParallel::getMyHostID() {
  return _hostname_hostid_map[_hostname];
}

int nestedParallel::getMyRank() {
  return _my_rank;
}

int nestedParallel::getMyRankInSharedMemoryComm() {
  return _my_rank_shmem;
}

bool nestedParallel::isDistributedNode() {
  if( getMyDistributedMemoryMPIComm() == MPI_COMM_NULL )
    return false; 
  else 
    return true;
}

nestedParallel::~nestedParallel() {
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Group_free( &(_intera_node_mpigroup_map[getMyHostID()]) );
  MPI_Comm_free( &(_intera_node_mpicomm_map[getMyHostID()]) );
  if( isDistributedNode() == true ) {
    MPI_Group_free( &_inter_node_mpigroup );
    MPI_Comm_free( &_inter_node_mpicomm );
  }
  MPI_Finalize(); 
}

