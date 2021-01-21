#include <iostream>
#include <cstdint>
#include <vector>
#include <chrono>
#include <mpi.h>
#include "lodepng_old/lodepng.h"

int main(int nargs, char* argv[])
{
    MPI_Init( &nargs, &argv );
    MPI_Comm globComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
    int nbp, rank;
    MPI_Comm_size(globComm, &nbp);
    MPI_Comm_rank(globComm, &rank);
    
    int range; // Degré voisinage (par défaut 1)
    int nb_cases; // 2^(2*range+1)
    std::int64_t dim; // Nombre de cellules prises pour la simultation
    std::int64_t nb_iter;

    range = 1;
    if ( nargs > 1 ) range = std::stoi(argv[1]);

    nb_cases = (1<<(2*range+1));
    dim = 1000;
    if ( nargs > 2 ) dim = std::stoll(argv[2]);
    nb_iter = dim;
    if ( nargs > 3 ) nb_iter = std::stoll(argv[3]);

    std::cout << "Resume des parametres : " << std::flush << std::endl;
    std::cout << "\tNombre de cellules  : " << dim << std::flush << std::endl;
    std::cout << "\tNombre d'iterations : " << nb_iter << std::flush << std::endl;
    std::cout << "\tDegre de voisinage  : " << range << std::flush << std::endl;
    std::cout << "\tNombre de cas       : " << (1ULL<<nb_cases) << std::flush << std::endl << std::endl;

    double chrono_calcul = 0.;
    double chrono_png    = 0.;
    
    
    
    
    if ((1ULL<<nb_cases)%nbp != 0)
    {
        std::cerr << "Le nombre de cases n'est pas divisible par nbp." << std::endl ;
        MPI_Abort(globComm,0);
    }
    
    /*std::vector<int> pixels;
    std::vector<int> pixels_loc(W*H / nbp);
    int ibloc = nbp-rank-1;
    int imin_loc = H / nbp * ibloc;
    int imax_loc = H / nbp * (ibloc+1);
    int H_loc = H / nbp;*/
    
    /*for ( int i = imin_loc; i < imax_loc; ++i ) {
        computeMandelbrotSetRow(W, H, maxIter, i, pixels_loc.data() + W*(imax_loc-i-1) );
    }*/
    
    /*if (rank == 0) {
        pixels.resize(W*H);
        std::cout << "gathering" << std::endl;
    }*/

    
    for ( std::int64_t num_config = 0; num_config < (1LL<<nb_cases); ++num_config)
    {
        auto start = std::chrono::steady_clock::now();
        std::uint64_t nb_cells = dim+2*range;// Les cellules avec les conditions limites (toujours à zéro en séquentiel)
        std::vector<std::uint8_t> cells(nb_iter*nb_cells, 0); // le tableau 2D qui contient tous les états
        cells[nb_cells/2] = 1; // condition initiale
        
        // Faire des cells plus petits ici :
        std::uint64_t nb_cells_loc = (dim+2*range)/nbp;
        std::vector<std::uint8_t> cells_loc(nb_iter*nb_cells_loc, 0);
        std::vector<std::uint8_t> cells_loc_bis(nb_iter*nb_cells_loc, 0);
        
        if (rank != 0){
            cells_loc_bis[0] = cells_loc[nb_cells_loc];
            cells_loc = cells_loc_bis;
        }
        
        for ( std::int64_t iter = 1; iter < nb_iter; ++iter )
        {
            for ( std::int64_t i = range; i < range + dim; ++i )
            {
                int val = 0;
                for ( std::int64_t j = i-range; j <= i+range; j++ )
                {
                    val = 2*val + cells_loc[(iter-1)*nb_cells_loc + j];
                }
                val = (1<<val);
                if ((val&num_config) == 0) 
                    cells_loc[iter*nb_cells_loc + i] = 0;
                else 
                {
                    cells_loc[iter*nb_cells_loc + i] = 1;
                }
            }
        }
                
        MPI_Gather(cells_loc.data(), nb_cells_loc, MPI_INT,       // emetteurs
                   cells.data(), nb_cells_loc, MPI_INT,           // receveur, bien mettre 2 fois la même taille!
                   0,                                             // root process
                   globComm );
        
        auto end = std::chrono::steady_clock::now();    
        std::chrono::duration<double> elapsed_seconds = end-start;
        chrono_calcul += elapsed_seconds.count();
        // Sauvegarde de l'image :
        start = std::chrono::steady_clock::now();
        std::vector<std::uint8_t> image(4*nb_iter*dim, 0xFF);
        for ( std::int64_t iter = 0; iter < nb_iter; iter ++ )
        {
            for ( std::int64_t i = range; i < range+dim; i ++ )
            {
                if (cells[iter*nb_cells + i] == 1) 
                {
                    image[4*iter*dim + 4*(i-range) + 0 ] = 0;
                    image[4*iter*dim + 4*(i-range) + 1 ] = 0;
                    image[4*iter*dim + 4*(i-range) + 2 ] = 0;
                    image[4*iter*dim + 4*(i-range) + 3 ] = 0xFF;
                }
            }
        }
        end = std::chrono::steady_clock::now();    
        char filename[256];
        sprintf(filename, "config%03lld.png",num_config);
        lodepng::encode(filename, image.data(), dim, nb_iter);  // ligne a commenter pour supprimer la sauvegarde des fichiers
        elapsed_seconds = end-start;
        chrono_png += elapsed_seconds.count();
    }
    
    if (rank==0){
        std::cout << "Temps mis par le calcul : " << chrono_calcul << " secondes." << std::flush << std::endl;
        std::cout << "Temps mis pour constituer les images : " << chrono_png << " secondes." << std::flush << std::endl;
    }
    MPI_Finalize();
    return EXIT_SUCCESS;
}
