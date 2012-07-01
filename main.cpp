#include "nestedParallel.h"

int main( int nargs , char *args[] ) {
  nestedParallel np( &nargs , &args );
  std::cout << "My rank = " << np.getMyRank() << "  ";
  std::cout << np.getMyDOTColor() << "  ";
  if( np.getMyDistributedMemoryMPIComm() != MPI_COMM_NULL )
    std::cout << " distributed node ";
  else
    std::cout << " non-dist node ";
  std::cout << "Shared memory mpi_comm_rank = " << np.getMyRankInSharedMemoryComm() << "\n";
  return 0;
}

