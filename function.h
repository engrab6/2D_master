//*************************CALCULS MACROSCOPIQUES**************************//
double pscal(double *a,double *b,  int D);
void density ( int j, int Q, Lattice lat, double sigma);
void velocity( int j, int D,  int Q, double** xi, Lattice lat);


//*************************GEOMETRIE DU DOMAINE**************************//

//Coordonnées des lattices
void localisation(int nx, int ny, double dx, double** position);
//Matrice de connectivité
void connectivite(int nx,int ny,  int Q, int** conn);
//Donne les directions opposées aux populations considérés (utile pour le HWBB)
void bounceback_neighbour( int* bb,  int Q);
//Cas pour les frontières du domaine et/ou solides
void domainCondition(int nx, int ny,  int* cas);
void solidCondition(int Q, int** conn,int nx, int ny, int*cas, bool* typeLat);


//********************************SOLIDES********************************//

//Donne la position min et max des lattices solides
void pos_solide (bool* typeLat,  int* pos, int nx, int ny);
//CYLINDRE CARRE
//Matrice des 4 coins de la section carrée du cylindre
void SquareCylinder(double abscisse, double ordonnee, double diametre, double** coin);
//Remplit typeLat pour un cylindre carré
void typeSquare( int N, double** coin, double** position, bool* typeLat);

