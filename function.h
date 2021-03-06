//*************************CALCULS MACROSCOPIQUES**************************//
double pscal(double *a,double *b,  int D, double sigma);
void density ( int j, int Q, Lattice lat, double sigma);
void velocity( int j, int D,  int Q, double** xi, Lattice lat, double sigma);

//***************************EQUILIBRE**************************************//
double*** Qi_function (int k, double cs, double** xi, int D, int Q);
void fi_equilibre (int j, int k, double rho, double cs, Lattice lat, double* u, double** xi, int D, double*** Qi, double* buffer, double* omega_i,double sigma);
void PI_neq_inlet(double** Pi_neq, int j, int Q, Lattice lat, double*** Qi, double* buffer, int* bb);
void PI_neq_outlet(double** Pi_neq, int j, int Q, Lattice lat, double*** Qi, double* buffer, int* bb);
void fi_bar(double* omega_i, double***Qi, double** Pi_neq, double cs, int j, int D, Lattice lat, double* buffer, int Q);

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
void typeCircular(double abscisse, double ordonnee, double diametre, double** coin, int N, double** position, bool* typeLat);

//**************************WALL FUNCTION******************************//
//Donne la valeur de Ei(x)
double Ei_big(int n, double x);

//**********************UTILE POUR L'EXPORTATION****************//
char FileName(double Kn);

//**********************Caractéristiques du milieu************//
double porosity( bool* typeLat, int N);
