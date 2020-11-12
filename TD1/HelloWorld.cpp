# include <iostream>
# include <cstdlib>
# include <mpi.h> // important pour MPI
# include <sstream>
# include <fstream>

int main( int nargs, char* argv[] )
{
    MPI_Init(&nargs, &argv); // indique que ce sera un programme MPI, bien passer les param
    
    MPI_Comm globComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
    int nbp;
    MPI_Comm_size(globComm, &nbp);
    
    int numero_du_processus, nombre_de_processus; // toujours inclure

    MPI_Comm_rank(MPI_COMM_WORLD,
                  &numero_du_processus); // rang dans le proco
    
    MPI_Comm_size(MPI_COMM_WORLD,
                  &nombre_de_processus); // nb de proco total
    
    /*std::cout << "Hello world from "
              << numero_du_processus << " in "
              << nombre_de_processus << " executed"
              << std::endl;*/
    
    int rank;
    MPI_Comm_rank(globComm, &rank);
    // Création d'un fichier pour ma propre sortie en écriture :
    std::stringstream fileName;
    fileName << "Output" << std::setfill('0') << std::setw(5) << rank << ".txt";
    std::ofstream output( fileName.str().c_str() );
    
    output << "I'm the processus " << rank << " on " << nbp << " processes." << std::endl;

    output.close();
    
    MPI_Finalize(); // pas oublier sq on a fini la dernière fct parallèle
    return EXIT_SUCCESS; // comme return 0;
}
