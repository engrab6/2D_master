// C++ Standard includes
#include <iostream> 
#include <cmath>
#include <ctime>
#include <stdio.h>
#include <algorithm>

// Local includes
#include "domain.h"
#include "lattice.h"
#include "solutionExporterEulerian.h"
#include "boundaryc.h"
#include "function.h"

#define M_PI  3.14159265358979323846



//Fonction de propagation des populations
//f(x+dx,t+dt) = f*(x,t)
void propagation( int const& j, Lattice lat, double** f_star, bool* typeLat, int** conn, int* bb, int Q)
{
	for (int k=0;k<Q;k++)
	{
		//Condition pour qu'il y ait propagation : pour la lattice j considérée
		//si son voisin bb[k] (à l'opposé de k) ne sort pas du domaine
		//si la lattice est une lattice fluide
		//si son voisin bb[k] est une lattice fluide
		//Alors on peut propager
		if(conn[j][bb[k]]!=-1 && !typeLat[j] && !typeLat[conn[j][bb[k]]])
		{
			lat.f_[j][k] = f_star[conn[j][bb[k]]][k];
		}
	}
}

//Fonction de traitement des conditions aux frontières : conditions périodiques Nord/Sud
void periodic_NS_BC( int j,int nx, int ny,  int cas, Lattice lat, double** f_star)
{
	//En fonction du cas de la cellule
	switch (cas)
	{
		case 3: //Cas NORD
		lat.f_[j][4] = f_star[j%nx][4]; //Condition périodique en entrée
		lat.f_[j][7] = f_star[j%nx+1][7];
		lat.f_[j][8] = f_star[j%nx-1][8];
		break;
		case 4: //Cas SUD
		lat.f_[j][6] = f_star[nx*ny-nx+j+1][6]; //Condition périodique en sortie
		lat.f_[j][2] = f_star[nx*ny-nx+j][2];
		lat.f_[j][5] = f_star[nx*ny-nx+j-1][5];
		break;
		/*case 5: //Cas SUD-OUEST
		lat.f_[j][2] = f_star[nx*ny-nx][2]; //Conditions périodique en sortie
		lat.f_[j][6] = f_star[nx*ny-nx+1][6];
		lat.f_[j][5] = f_star[nx*ny-1][5];
		break;
		case 6://Cas SUD-EST
		lat.f_[j][2] = f_star[nx*ny-1][2]; //Condition périodique en sortie
		lat.f_[j][5] = f_star[nx*ny-2][5];
		lat.f_[j][8] = f_star[nx*ny-nx][8];
		break;
		case 7://Cas NORD-OUEST
		lat.f_[j][4] = f_star[0][4];//Condition périodique en entrée
		lat.f_[j][7] = f_star[1][7];
		lat.f_[j][8] = f_star[nx-1][8];
		break;//Cas NORD-EST
		case 8:
		lat.f_[j][4] = f_star[nx-1][4]; //Condition périodique en entrée
		lat.f_[j][8] = f_star[nx-2][8];
		lat.f_[j][7] = f_star[0][7];
		break;*/
	}
}

//Fonction de traitement des conditions aux frontières : conditions périodiques Ouest/Est
void periodic_WE_BC( int j,int nx, int ny,  int cas, Lattice lat, double** f_star)
{
	//En fonction du cas de la cellule
	switch (cas)
	{
		case 1: //Cas OUEST
		lat.f_[j][1] = f_star[j+nx-1][1]; 
		lat.f_[j][5] = f_star[j-1][5];
		lat.f_[j][8] = f_star[j+2*nx-1][8];
		break;
		case 2: //Cas EST
		lat.f_[j][3] = f_star[j-nx+1][3]; 
		lat.f_[j][6] = f_star[j-2*nx+1][6];
		lat.f_[j][7] = f_star[j+1][7];
		break;
		case 5://Cas SUD-OUEST
		lat.f_[j][1] = f_star[nx-1][1];
		lat.f_[j][8] = f_star[2*nx-1][8];
		lat.f_[j][5] = f_star[nx*ny-1][5];
		break;
		case 6://Cas SUD-EST
		lat.f_[j][3] = f_star[0][3]; 
		lat.f_[j][7] = f_star[nx][7];
		lat.f_[j][6] = f_star[nx*ny-nx][6];
		break;
		case 7://Cas NORD-OUEST
		lat.f_[j][1] = f_star[nx*ny-1][1];
		lat.f_[j][5] = f_star[j-1][5];
		lat.f_[j][8] = f_star[nx-1][8];
		break;
		case 8://Cas NORD-EST
		lat.f_[j][3] = f_star[j-nx+1][3];
		lat.f_[j][6] = f_star[j-2*nx+1][6];
		lat.f_[j][7] = f_star[0][7];
		break;
	}
}

//Fonction de traitement des conditions aux frontières : conditions périodiques Ouest/Est AVEC gradient de pression (type Zhang et Kwok (2006))
//Le sigma est déterminé grâce aux conditions suivantes :
//Même vitesse entre l'entrée et la sortie
//Gradient de pression entre l'entrée et la sortie égale à dRHO*cs*cs (car P = rho*cs²)
void periodic_pressure_WE_BC (int j, int nx, int ny, int cas, Lattice lat, double** f_star, double dRHO, double sigma, double cs)
{
	
		switch (cas)
	{
		case 1: //Cas OUEST
		sigma = 1./3*(dRHO-1 -(lat.f_[j][0]+lat.f_[j][2]+lat.f_[j][4]-(lat.f_[j+nx-1][0]+lat.f_[j+nx-1][2]+lat.f_[j+nx-1][4])));
		lat.f_[j][1] = lat.f_[j+nx-1][1] + sigma; 
		lat.f_[j][5] = lat.f_[j+nx-1][5] + 0.25*sigma;
		lat.f_[j][8] = lat.f_[j+nx-1][8] + 0.25*sigma;
		break;
		case 2: //Cas EST
		sigma = 1./3*(dRHO -1-(lat.f_[j-nx+1][0]+lat.f_[j-nx+1][2]+lat.f_[j-nx+1][4]-(lat.f_[j][0]+lat.f_[j][2]+lat.f_[j][4])));
		lat.f_[j][3] = lat.f_[j-nx+1][3] - sigma;
		lat.f_[j][6] = lat.f_[j-nx+1][6] - 0.25*sigma;
		lat.f_[j][7] = lat.f_[j-nx+1][7] - 0.25*sigma;
		break;
		case 5://Cas SUD-OUEST
		sigma = 1./3*(dRHO -1-(lat.f_[j][0]+lat.f_[j][2]+lat.f_[j][4]-(lat.f_[j+nx-1][0]+lat.f_[j+nx-1][2]+lat.f_[j+nx-1][4])));
		lat.f_[j][1] = lat.f_[j+nx-1][1] + sigma; 
		lat.f_[j][5] = lat.f_[j+nx-1][5] + 0.25*sigma;
		lat.f_[j][8] = lat.f_[j+nx-1][8] + 0.25*sigma;
		break;
		case 6://Cas SUD-EST
		sigma = 1./3*(dRHO -1-(lat.f_[j-nx+1][0]+lat.f_[j-nx+1][2]+lat.f_[j-nx+1][4]-(lat.f_[j][0]+lat.f_[j][2]+lat.f_[j][4])));
		lat.f_[j][3] = lat.f_[j-nx+1][3] - sigma;
		lat.f_[j][6] = lat.f_[j-nx+1][6] - 0.25*sigma;
		lat.f_[j][7] = lat.f_[j-nx+1][7] - 0.25*sigma;
		break;
		case 7://Cas NORD-OUEST
		sigma = 1./3*(dRHO -1-(lat.f_[j][0]+lat.f_[j][2]+lat.f_[j][4]-(lat.f_[j+nx-1][0]+lat.f_[j+nx-1][2]+lat.f_[j+nx-1][4])));
		lat.f_[j][1] = lat.f_[j+nx-1][1] + sigma;
		lat.f_[j][5] = lat.f_[j+nx-1][5] + 0.25*sigma;
		lat.f_[j][8] = lat.f_[j+nx-1][8] + 0.25*sigma;
		break;
		case 8://Cas NORD-EST
		sigma = 1./3*(dRHO-1 -(lat.f_[j-nx+1][0]+lat.f_[j-nx+1][2]+lat.f_[j-nx+1][4]-(lat.f_[j][0]+lat.f_[j][2]+lat.f_[j][4])));
		lat.f_[j][3] = lat.f_[j-nx+1][3] - sigma;
		lat.f_[j][6] = lat.f_[j-nx+1][6] - 0.25*sigma;
		lat.f_[j][7] = lat.f_[j-nx+1][7] - 0.25*sigma;
		break;
	}
}


//Fonction de traitement des conditions limites : Halfway bounce-back pour les solides DANS LE DOMAINE
void bounceback_solid_BC(int nx, int const& j, Lattice lat, double** f_star, int** const& conn, bool*  typeLat,  int* const& bb, double& nombre, int* pos, int cas) //Cas spéciaux pour le solide (coins et voisins)
{
	for (int k=0;k<9;k++)
	{	
		//Condition pour le bounce-back DANS le domaineSi
		//Si le voisin dans la direction du bounce-back n'est pas hors du domaine
		//Si la lattice j est une lattice fluide
		//Si le voisin dans la direction du bounce-back est une lattice solide
		//Alors il y a BB
		// Attention ! On ne fait pas le contraire (si la lattice est solide, et que le voisin est fluide, alors on effectue un traitement sur les populations du voisin fluide)
		//Car pour des mur convexes, il peut y avoir redondance
		if ( conn[j][k]!=-1 && !typeLat[j] && typeLat[conn[j][k]])
		{
			lat.f_[j][bb[k]] = f_star[j][k];
		}
	}
}


//Fonction de traitement des conditions limites : CBBSR pour les solides DANS LE DOMAINE
void CBBSR_solid_square_BC(int nx, int const& j, Lattice lat, double** f_star, int** const& conn, bool*  typeLat, double r, int* pos, int tabVoisin)
{
	switch (tabVoisin)
	{
		case 100: //Frontière Sud du solide
		lat.f_[j][4] = f_star[j][2];
		lat.f_[j][7] = r*f_star[j][6] + (1-r)*f_star[j][5];
		lat.f_[j][8] = r*f_star[j][5] + (1-r)*f_star[j][6];
		break;
		case 200: //Frontière Est
		lat.f_[j][1] = f_star[j][2];
		lat.f_[j][5] = r*f_star[j][6] + (1-r)*f_star[j][5];
		lat.f_[j][8] = r*f_star[j][5] + (1-r)*f_star[j][6];
		break;
		case 400: //Frontière Nord
		lat.f_[j][2] = f_star[j][2];
		lat.f_[j][5] = r*f_star[j][6] + (1-r)*f_star[j][5];
		lat.f_[j][6] = r*f_star[j][5] + (1-r)*f_star[j][6];
		break;
		case 290: //Frontière Ouest
		lat.f_[j][3] = f_star[j][2];
		lat.f_[j][6] = r*f_star[j][6] + (1-r)*f_star[j][5];
		lat.f_[j][7] = r*f_star[j][5] + (1-r)*f_star[j][6];
		break;
		case 32: //Coin SO
		lat.f_[j][7] = r*f_star[j][5] + (1-r)*f_star[j][6];
		//lat.f_[j][7] = r*f_star[j][5] + (1-r)*f_star[j][8];
		//lat.f_[j][7] = f_star[j][5];
		break;
		case 64: //Coin SE
		lat.f_[j][8] = r*f_star[j][6] + (1-r)*f_star[j][7];
		//lat.f_[j][8] = r*f_star[j][6] + (1-r)*f_star[j][5];
		//lat.f_[j][8] = f_star[j][6];
		break;
		case 128: //Coin NE
		lat.f_[j][5] = r*f_star[j][7]+(1-r)*f_star[j][6];
		//lat.f_[j][5] = r*f_star[j][7]+(1-r)*f_star[j][8];
		//lat.f_[j][5] = f_star[j][7];
		break;
		case 256: //Coin NO
		lat.f_[j][6] = r*f_star[j][8]+(1-r)*f_star[j][5];
		//lat.f_[j][6] = r*f_star[j][8]+(1-r)*f_star[j][7];		
		//lat.f_[j][6] = f_star[j][8];
		break;
		case 36: //Coin SO-E
		lat.f_[j][4] = f_star[j][2];
		lat.f_[j][7] = r*f_star[j][8] + (1-r)*f_star[j][6];		
		break;
		case 68: //Coin SE-O
		lat.f_[j][4] = f_star[j][2];
		lat.f_[j][8] = r*f_star[j][6] + (1-r)*f_star[j][5];	
		break;
		case 72: //Coin SO-N
		lat.f_[j][1] = f_star[j][3];
		lat.f_[j][8] = r*f_star[j][6] + (1-r)*f_star[j][7];	
		break;
		case 136: //Coin NE-S
		lat.f_[j][1] = f_star[j][3];
		lat.f_[j][5] = r*f_star[j][7] + (1-r)*f_star[j][6];	
		break;
		case 144: //Coin NE-O
		lat.f_[j][2] = f_star[j][4];
		lat.f_[j][5] = r*f_star[j][7] + (1-r)*f_star[j][8];	
		break;
		case 272: //Coin NO-E
		lat.f_[j][2] = f_star[j][4];
		lat.f_[j][6] = r*f_star[j][8] + (1-r)*f_star[j][7];	
		break;
		case 258: //Coin NO-S
		lat.f_[j][3] = f_star[j][1];
		lat.f_[j][6] = r*f_star[j][8] + (1-r)*f_star[j][5];	
		break;
		case 34: //Coin SO-N
		lat.f_[j][3] = f_star[j][1];
		lat.f_[j][7] = r*f_star[j][5] + (1-r)*f_star[j][6];	
		break;
		
		
	}
}

//Fonction de traitement des conditions d'entrée sortie : Pression imposée en entrée, type ZOU-HE
void pression_in_BC( int j,  int cas, Lattice lat, double xi_r, double rho_in)
{
	double u_x;
	switch (cas)
	{
		case 1: //Ouest
		u_x = xi_r-xi_r/rho_in*(lat.f_[j][0]+lat.f_[j][2]+lat.f_[j][4]+2*(lat.f_[j][3]+lat.f_[j][6]+lat.f_[j][7]));
		lat.f_[j][1] = lat.f_[j][3]+2/(3*xi_r)*rho_in*u_x;
		lat.f_[j][5] = lat.f_[j][7]-0.5*(lat.f_[j][2]-lat.f_[j][4])+1/(6*xi_r)*rho_in*u_x ;//+ 1/(2*xi_r)*rho_in*lat.u_[j][1];
		lat.f_[j][8] = lat.f_[j][6]+0.5*(lat.f_[j][2]-lat.f_[j][4])+1/(6*xi_r)*rho_in*u_x ;//- 1/(2*xi_r)*rho_in*lat.u_[j][1];
		break;
		case 5: //Coin Sud-Ouest
		lat.f_[j][1] = lat.f_[j][3];
		lat.f_[j][8] = 0.5*(rho_in-(lat.f_[j][0]+lat.f_[j][1]+lat.f_[j][2]+lat.f_[j][3]+lat.f_[j][4]+lat.f_[j][5]+lat.f_[j][7]));
		lat.f_[j][6] = 0.5*(rho_in-(lat.f_[j][0]+lat.f_[j][1]+lat.f_[j][2]+lat.f_[j][3]+lat.f_[j][4]+lat.f_[j][5]+lat.f_[j][7]));
		/*u_x = xi_r-xi_r/rho_in*(lat.f_[j][0]+lat.f_[j][2]+lat.f_[j][4]+2*(lat.f_[j][3]+lat.f_[j][6]+lat.f_[j][7]));
		lat.f_[j][1] = lat.f_[j][3]+2/(3*xi_r)*rho_in*u_x;
		lat.f_[j][5] = lat.f_[j][7]-0.5*(lat.f_[j][2]-lat.f_[j][4])+1/(6*xi_r)*rho_in*u_x ;//+ 1/(2*xi_r)*rho_in*lat.u_[j][1];
		lat.f_[j][8] = lat.f_[j][6]+0.5*(lat.f_[j][2]-lat.f_[j][4])+1/(6*xi_r)*rho_in*u_x ;// - 1/(2*xi_r)*rho_in*lat.u_[j][1];*/
		break;
		case 7: //Coin Nord-Ouest
		lat.f_[j][1] = lat.f_[j][3];
		lat.f_[j][7] = 0.5*(rho_in-(lat.f_[j][0]+lat.f_[j][1]+lat.f_[j][2]+lat.f_[j][3]+lat.f_[j][4]+lat.f_[j][6]+lat.f_[j][8]));
		lat.f_[j][5] =0.5*(rho_in-(lat.f_[j][0]+lat.f_[j][1]+lat.f_[j][2]+lat.f_[j][3]+lat.f_[j][4]+lat.f_[j][6]+lat.f_[j][8]));
		/*u_x = xi_r-xi_r/rho_in*(lat.f_[j][0]+lat.f_[j][2]+lat.f_[j][4]+2*(lat.f_[j][3]+lat.f_[j][6]+lat.f_[j][7]));
		lat.f_[j][1] = lat.f_[j][3]+2/(3*xi_r)*rho_in*u_x;
		lat.f_[j][5] = lat.f_[j][7]-0.5*(lat.f_[j][2]-lat.f_[j][4])+1/(6*xi_r)*rho_in*u_x ;//+ 1/(2*xi_r)*rho_in*lat.u_[j][1];
		lat.f_[j][8] = lat.f_[j][6]+0.5*(lat.f_[j][2]-lat.f_[j][4])+1/(6*xi_r)*rho_in*u_x ;//- 1/(2*xi_r)*rho_in*lat.u_[j][1];*/
		break;
	}
}

//Fonction de traitement des conditions d'entrée sortie : Vitesse imposée en entrée, type ZOU-HE
void vitesse_in_BC( int j,int nx,int cas, Lattice lat, double xi_r, double** v_in)
{
	switch (cas)
	{
		case 1: //Ouest
		lat.rho_[j] = xi_r/(xi_r-v_in[j/nx][0])*(lat.f_[j][0]+lat.f_[j][2]+lat.f_[j][4]+2*(lat.f_[j][3]+lat.f_[j][6]+lat.f_[j][7]));
		lat.f_[j][1] = lat.f_[j][3] + 2/(3*xi_r)*lat.rho_[j]*v_in[j/nx][0];
		lat.f_[j][5] = lat.f_[j][7] - 0.5*(lat.f_[j][2]-lat.f_[j][4]) + 1/(6*xi_r)*lat.rho_[j]*v_in[j/nx][0] ;//+ 0.5*lat.rho_[j]/xi_r*lat.u_[j][1]; 
		lat.f_[j][8] = lat.f_[j][6] + 0.5*(lat.f_[j][2]-lat.f_[j][4]) + 1/(6*xi_r)*lat.rho_[j]*v_in[j/nx][0] ;//- 0.5*lat.rho_[j]/xi_r*lat.u_[j][1]; 
		break;
		case 5: //Coin Sud-Ouest
		lat.f_[j][1] = lat.f_[j][3];
		lat.f_[j][8] = 0.5*(lat.rho_[j+nx]-(lat.f_[j][0]+lat.f_[j][1]+lat.f_[j][2]+lat.f_[j][3]+lat.f_[j][4]+lat.f_[j][5]+lat.f_[j][7]));
		lat.f_[j][6] = 0.5*(lat.rho_[j+nx]-(lat.f_[j][0]+lat.f_[j][1]+lat.f_[j][2]+lat.f_[j][3]+lat.f_[j][4]+lat.f_[j][5]+lat.f_[j][7]));
		/*lat.rho_[j] = xi_r/(xi_r-v_in[j/nx][0])*(lat.f_[j][0]+lat.f_[j][2]+lat.f_[j][4]+2*(lat.f_[j][3]+lat.f_[j][6]+lat.f_[j][7]));
		lat.f_[j][1] = lat.f_[j][3] + 2/(3*xi_r)*lat.rho_[j]*v_in[j/nx][0];
		lat.f_[j][5] = lat.f_[j][7] - 0.5*(lat.f_[j][2]-lat.f_[j][4]) + 1/(6*xi_r)*lat.rho_[j]*v_in[j/nx][0] ;//+ 0.5*lat.rho_[j]/xi_r*lat.u_[j][1];
		lat.f_[j][8] = lat.f_[j][6] + 0.5*(lat.f_[j][2]-lat.f_[j][4]) + 1/(6*xi_r)*lat.rho_[j]*v_in[j/nx][0] ;//- 0.5*lat.rho_[j]/xi_r*lat.u_[j][1] ;*/
		
		break;
		case 7: //Coin Nord-Ouest
		lat.f_[j][1] = lat.f_[j][3];
		lat.f_[j][7] = 0.5*(lat.rho_[j-nx]-(lat.f_[j][0]+lat.f_[j][1]+lat.f_[j][2]+lat.f_[j][3]+lat.f_[j][4]+lat.f_[j][6]+lat.f_[j][8]));
		lat.f_[j][5] =0.5*(lat.rho_[j-nx]-(lat.f_[j][0]+lat.f_[j][1]+lat.f_[j][2]+lat.f_[j][3]+lat.f_[j][4]+lat.f_[j][6]+lat.f_[j][8]));
		/*lat.rho_[j] = xi_r/(xi_r-v_in[j/nx][0])*(lat.f_[j][0]+lat.f_[j][2]+lat.f_[j][4]+2*(lat.f_[j][3]+lat.f_[j][6]+lat.f_[j][7]));
		lat.f_[j][1] = lat.f_[j][3] + 2/(3*xi_r)*lat.rho_[j]*v_in[j/nx][0];
		lat.f_[j][5] = lat.f_[j][7] - 0.5*(lat.f_[j][2]-lat.f_[j][4]) + 1/(6*xi_r)*lat.rho_[j]*v_in[j/nx][0] ;//+ 0.5*lat.rho_[j]/xi_r*lat.u_[j][1];
		lat.f_[j][8] = lat.f_[j][6] + 0.5*(lat.f_[j][2]-lat.f_[j][4]) + 1/(6*xi_r)*lat.rho_[j]*v_in[j/nx][0] ;//- 0.5*lat.rho_[j]/xi_r*lat.u_[j][1];*/

		break;
	}
}

//Fonction de traitement des conditions d'entrée sortie : Pression imposée en sortie, type ZOU-HE
void pression_out_BC( int j,  int cas, Lattice lat, double xi_r,double rho_out)
{
	double u_x;
	switch (cas)
	{
		case 2: //Est
		u_x = xi_r/rho_out*(lat.f_[j][0]+lat.f_[j][2]+lat.f_[j][4]+2*(lat.f_[j][1]+lat.f_[j][5]+lat.f_[j][8]))-xi_r;
		lat.f_[j][3] = lat.f_[j][1] - 2/(3*xi_r)*rho_out*u_x;
		lat.f_[j][7] = lat.f_[j][5] + 0.5*(lat.f_[j][2]-lat.f_[j][4]) - 1/(6*xi_r)*rho_out*u_x ;//- 1/(2*xi_r)*rho_out*lat.u_[j][1];
		lat.f_[j][6] = lat.f_[j][8] - 0.5*(lat.f_[j][2]-lat.f_[j][4]) - 1/(6*xi_r)*rho_out*u_x ;//+ 1/(2*xi_r)*rho_out*lat.u_[j][1];
		break;
		case 6: //Coin Sud-Est
		lat.f_[j][3] = lat.f_[j][1];
		lat.f_[j][5] = 0.5*(rho_out-(lat.f_[j][0]+lat.f_[j][1]+lat.f_[j][2]+lat.f_[j][3]+lat.f_[j][4]+lat.f_[j][6]+lat.f_[j][8]));
		lat.f_[j][7] = 0.5*(rho_out-(lat.f_[j][0]+lat.f_[j][1]+lat.f_[j][2]+lat.f_[j][3]+lat.f_[j][4]+lat.f_[j][6]+lat.f_[j][8]));
		u_x = xi_r/rho_out*(lat.f_[j][0]+lat.f_[j][2]+lat.f_[j][4]+2*(lat.f_[j][1]+lat.f_[j][5]+lat.f_[j][8]))-xi_r;
		/*lat.f_[j][3] = lat.f_[j][1] - 2/(3*xi_r)*rho_out*u_x;
		lat.f_[j][7] = lat.f_[j][5] + 0.5*(lat.f_[j][2]-lat.f_[j][4]) - 1/(6*xi_r)*rho_out*u_x ;//- 1/(2*xi_r)*rho_out*lat.u_[j][1];
		lat.f_[j][6] = lat.f_[j][8] - 0.5*(lat.f_[j][2]-lat.f_[j][4]) - 1/(6*xi_r)*rho_out*u_x ;//+ 1/(2*xi_r)*rho_out*lat.u_[j][1];*/
		
		break;
		case 8: //Coin Nord-Est
		lat.f_[j][3] = lat.f_[j][1];
		lat.f_[j][6] = 0.5*(rho_out-(lat.f_[j][0]+lat.f_[j][1]+lat.f_[j][2]+lat.f_[j][3]+lat.f_[j][4]+lat.f_[j][5]+lat.f_[j][7]));
		lat.f_[j][8] = 0.5*(rho_out-(lat.f_[j][0]+lat.f_[j][1]+lat.f_[j][2]+lat.f_[j][3]+lat.f_[j][4]+lat.f_[j][5]+lat.f_[j][7]));
		/*u_x = xi_r/rho_out*(lat.f_[j][0]+lat.f_[j][2]+lat.f_[j][4]+2*(lat.f_[j][1]+lat.f_[j][5]+lat.f_[j][8]))-xi_r;
		lat.f_[j][3] = lat.f_[j][1] - 2/(3*xi_r)*rho_out*u_x;
		lat.f_[j][7] = lat.f_[j][5] + 0.5*(lat.f_[j][2]-lat.f_[j][4]) - 1/(6*xi_r)*rho_out*u_x ;//- 1/(2*xi_r)*rho_out*lat.u_[j][1];
		lat.f_[j][6] = lat.f_[j][8] - 0.5*(lat.f_[j][2]-lat.f_[j][4]) - 1/(6*xi_r)*rho_out*u_x ;//+ 1/(2*xi_r)*rho_out*lat.u_[j][1];*/
		
		break;
	}
}

//Fonction de traitement des conditions d'entrée sortie : Vitesse imposée en sortie, type ZOU-HE
void vitesse_out_BC( int j,int nx,int cas, Lattice lat, double xi_r,double** v_out)
{
	switch (cas)
	{
		case 2: //Est 
		lat.rho_[j] = xi_r/(xi_r+v_out[j/nx][0])*(lat.f_[j][0]+lat.f_[j][2]+lat.f_[j][4]+2*(lat.f_[j][1]+lat.f_[j][5]+lat.f_[j][8]));
		lat.f_[j][3] = lat.f_[j][1] - 2/(3*xi_r)*lat.rho_[j]*lat.u_[j][0];
		lat.f_[j][7] = lat.f_[j][5] + 0.5*(lat.f_[j][2]-lat.f_[j][4]) - 1/(6*xi_r)*lat.rho_[j]*v_out[j/nx][0] ;//- 1/(2*xi_r)*lat.rho_[j]*lat.u_[j][1];
		lat.f_[j][6] = lat.f_[j][8] - 0.5*(lat.f_[j][2]-lat.f_[j][4]) - 1/(6*xi_r)*lat.rho_[j]*v_out[j/nx][0] ;//+ 1/(2*xi_r)*lat.rho_[j]*lat.u_[j][1]; 
		break;
		case 6: //Coin Sud-Est
		lat.rho_[j] = xi_r/(xi_r+v_out[j/nx][0])*(lat.f_[j][0]+lat.f_[j][2]+lat.f_[j][4]+2*(lat.f_[j][1]+lat.f_[j][5]+lat.f_[j][8]));
		lat.f_[j][3] = lat.f_[j][1] - 2/(3*xi_r)*lat.rho_[j]*lat.u_[j][0];
		lat.f_[j][7] = lat.f_[j][5] + 0.5*(lat.f_[j][2]-lat.f_[j][4]) - 1/(6*xi_r)*lat.rho_[j]*v_out[j/nx][0] ;//- 1/(2*xi_r)*lat.rho_[j]*lat.u_[j][1];
		lat.f_[j][6] = lat.f_[j][8] - 0.5*(lat.f_[j][2]-lat.f_[j][4]) - 1/(6*xi_r)*lat.rho_[j]*v_out[j/nx][0] ;//+ 1/(2*xi_r)*lat.rho_[j]*lat.u_[j][1]; 
		break;
		case 8: //Coin Nord-Est
		lat.rho_[j] = xi_r/(xi_r+v_out[j/nx][0])*(lat.f_[j][0]+lat.f_[j][2]+lat.f_[j][4]+2*(lat.f_[j][1]+lat.f_[j][5]+lat.f_[j][8]));
		lat.f_[j][3] = lat.f_[j][1] - 2/(3*xi_r)*lat.rho_[j]*lat.u_[j][0];
		lat.f_[j][7] = lat.f_[j][5] + 0.5*(lat.f_[j][2]-lat.f_[j][4]) - 1/(6*xi_r)*lat.rho_[j]*v_out[j/nx][0] ;//- 1/(2*xi_r)*lat.rho_[j]*lat.u_[j][1];
		lat.f_[j][6] = lat.f_[j][8] - 0.5*(lat.f_[j][2]-lat.f_[j][4]) - 1/(6*xi_r)*lat.rho_[j]*v_out[j/nx][0] ;//+ 1/(2*xi_r)*lat.rho_[j]*lat.u_[j][1]; 
		break;
	}
}
	
//Fonction de traitement des conditions aux frontières : Vitesse imposée sur la frontière Nord (pour créer une paroi mobile), type ZOU-HE
void driven_cavity_nord( int j,  int cas, Lattice lat, double xi_r,double v_e)
{
	switch (cas)
	{
		case 3: //Nord
		lat.rho_[j] = 1/(1+lat.u_[j][1]/xi_r)*(lat.f_[j][0]+lat.f_[j][1]+lat.f_[j][3]+2*(lat.f_[j][2]+lat.f_[j][5]+lat.f_[j][6]));
		lat.f_[j][4] = lat.f_[j][2]-2/(3*xi_r)*lat.rho_[j]*lat.u_[j][1];
		lat.f_[j][7] = lat.f_[j][5]+0.5*(lat.f_[j][1]-lat.f_[j][3])-0.5*lat.rho_[j]/xi_r*v_e-lat.rho_[j]/(6*xi_r)*lat.u_[j][1];
		lat.f_[j][8] = lat.f_[j][6]-0.5*(lat.f_[j][1]-lat.f_[j][3])+0.5*lat.rho_[j]/xi_r*v_e-lat.rho_[j]/(6*xi_r)*lat.u_[j][1];
		break;
		case 7: //Coin Nord-Ouest
		lat.rho_[j] = 1/(1+lat.u_[j][1]/xi_r)*(lat.f_[j][0]+lat.f_[j][1]+lat.f_[j][3]+2*(lat.f_[j][2]+lat.f_[j][5]+lat.f_[j][6]));
		lat.f_[j][4] = lat.f_[j][2]-2/(3*xi_r)*lat.rho_[j]*lat.u_[j][1];
		lat.f_[j][7] = lat.f_[j][5]+0.5*(lat.f_[j][1]-lat.f_[j][3])-0.5*lat.rho_[j]/xi_r*v_e-lat.rho_[j]/(6*xi_r)*lat.u_[j][1];
		lat.f_[j][8] = lat.f_[j][6]-0.5*(lat.f_[j][1]-lat.f_[j][3])+0.5*lat.rho_[j]/xi_r*v_e-lat.rho_[j]/(6*xi_r)*lat.u_[j][1];
		break;
		case 8: //Coin Nord-Est
		lat.rho_[j] = 1/(1+lat.u_[j][1]/xi_r)*(lat.f_[j][0]+lat.f_[j][1]+lat.f_[j][3]+2*(lat.f_[j][2]+lat.f_[j][5]+lat.f_[j][6]));
		lat.f_[j][4] = lat.f_[j][2]-2/(3*xi_r)*lat.rho_[j]*lat.u_[j][1];
		lat.f_[j][7] = lat.f_[j][5]+0.5*(lat.f_[j][1]-lat.f_[j][3])-0.5*lat.rho_[j]/xi_r*v_e-lat.rho_[j]/(6*xi_r)*lat.u_[j][1];
		lat.f_[j][8] = lat.f_[j][6]-0.5*(lat.f_[j][1]-lat.f_[j][3])+0.5*lat.rho_[j]/xi_r*v_e-lat.rho_[j]/(6*xi_r)*lat.u_[j][1];
		break;
	}
}

//Fonction de traitement des conditions aux frontières : Halfway bounce-back sur la frontière NORD
void bounceback_N_BC( int j, int cas, Lattice lat, double** f_star)
{
	switch (cas)
	{
		case 3: //N
		lat.f_[j][4] = f_star[j][2];
		lat.f_[j][8] = f_star[j][6];
		lat.f_[j][7] = f_star[j][5];
		break;
		case 7: //NO
		lat.f_[j][4] = f_star[j][2];
		lat.f_[j][8] = f_star[j][6];
		lat.f_[j][7] = f_star[j][5];
		break;
		case 8: //NE
		lat.f_[j][4] = f_star[j][2]; 	
		lat.f_[j][8] = f_star[j][6];
		lat.f_[j][7] = f_star[j][5];
		break;
	}
}

//Fonction de traitement des conditions aux frontières : Halfway bounce-back sur la frontière SUD
void bounceback_S_BC( int j, int cas, Lattice lat, double** f_star)
{
	switch (cas)
	{
		case 4: //S
		lat.f_[j][2] = f_star[j][4];
		lat.f_[j][5] = f_star[j][7];
		lat.f_[j][6] = f_star[j][8];
		break;
		case 5: //SO
		lat.f_[j][2] = f_star[j][4];
		lat.f_[j][5] = f_star[j][7];
		lat.f_[j][6] = f_star[j][8];
		break;
		case 6: //SE
		lat.f_[j][2] = f_star[j][4];
		lat.f_[j][5] = f_star[j][7];
		lat.f_[j][6] = f_star[j][8];
		break;
	}
}

//Fonction de traitement des conditions aux frontières : Halfway bounce-back sur la frontière EST
void bounceback_E_BC( int j, int cas, Lattice lat, double** f_star)
{
	switch (cas)
	{
		case 2://E
		lat.f_[j][3] = f_star[j][1];
		lat.f_[j][6] = f_star[j][8];
		lat.f_[j][7] = f_star[j][5];
		break;
		case 6://S-E
		lat.f_[j][3] = f_star[j][1];
		lat.f_[j][6] = f_star[j][8];
		lat.f_[j][7] = f_star[j][5];
		break;
		case 8://N-E
		lat.f_[j][3] = f_star[j][1];
		lat.f_[j][6] = f_star[j][8];
		lat.f_[j][7] = f_star[j][5];
		break;
	}
}

//Fonction de traitement des conditions aux frontières : Halfway bounce-back sur la frontière OUEST
void bounceback_W_BC( int j, int cas, Lattice lat, double** f_star)
{
	switch (cas)
	{
		case 1: //O
		lat.f_[j][1] = f_star[j][3];
		lat.f_[j][8] = f_star[j][6];
		lat.f_[j][5] = f_star[j][7];
		break;
		case 5://S-O
		lat.f_[j][1] = f_star[j][3];
		lat.f_[j][8] = f_star[j][6];
		lat.f_[j][5] = f_star[j][7];
		break;
		case 7://N-O
		lat.f_[j][1] = f_star[j][3];
		lat.f_[j][8] = f_star[j][6];
		lat.f_[j][5] = f_star[j][7];
		break;
	}
}

//Fonction de traitement des conditions aux frontières : Combined bounce-back specular reflection sur la frontière NORD
void CBBSR_N_BC(int j, int cas, Lattice lat, double r, double** f_star)
{
	switch(cas)
	{
		case 3: //N
		lat.f_[j][4] = f_star[j][2];
		lat.f_[j][8] = r*f_star[j][6] + (1-r)*f_star[j][5];
		lat.f_[j][7] = r*f_star[j][5] + (1-r)*f_star[j][6];
		break;
		case 7: //NO
		lat.f_[j][4] = f_star[j][2];
		lat.f_[j][8] = r*f_star[j][6] + (1-r)*f_star[j][5];
		lat.f_[j][7] = r*f_star[j][5] + (1-r)*f_star[j][6];
		break;
		case 8: //NE
		lat.f_[j][4] = f_star[j][2];
		lat.f_[j][8] = r*f_star[j][6] + (1-r)*f_star[j][5];
		lat.f_[j][7] = r*f_star[j][5] + (1-r)*f_star[j][6];
		break;
	}
}

//Fonction de traitement des conditions aux frontières : Combined bounce-back specular reflection sur la frontière NORD
void CBBSR_S_BC(int j, int cas, Lattice lat, double r, double** f_star)
{
	switch (cas)
	{
		case 4: //S
		lat.f_[j][2] = f_star[j][4];
		lat.f_[j][5] = r*f_star[j][7] + (1-r)*f_star[j][8];
		lat.f_[j][6] = r*f_star[j][8]+ (1-r)*f_star[j][7];
		break;
		case 5: //SO
		lat.f_[j][2] = f_star[j][4];
		lat.f_[j][5] = r*f_star[j][7] + (1-r)*f_star[j][8];
		lat.f_[j][6] = r*f_star[j][8]+ (1-r)*f_star[j][7];
		break;
		case 6: //SE
		lat.f_[j][2] = f_star[j][4];
		lat.f_[j][5] = r*f_star[j][7] + (1-r)*f_star[j][8];
		lat.f_[j][6] = r*f_star[j][8]+ (1-r)*f_star[j][7];
		break;
	}
}

//Fonction de traitement des conditions aux frontières : Combined bounce-back specular reflection sur la frontière NORD avec paroi mobile (pour un écoulement de Couette)
void CBBSR_N_BC_Couette(int j, int cas, Lattice lat, double r, double** f_star, double uw, double** xi, double cs, double* omega_i)
{
	switch(cas)
	{
		case 3: //N
		lat.f_[j][4] = f_star[j][2];
		lat.f_[j][8] = r*f_star[j][6] + (1-r)*f_star[j][5]+2*r*omega_i[8]*lat.rho_[j]*uw/(cs*cs)*xi[8][0];
		lat.f_[j][7] = r*f_star[j][5] + (1-r)*f_star[j][6]+2*r*omega_i[7]*lat.rho_[j]*uw/(cs*cs)*xi[7][0];
		break;
		case 7: //NO
		lat.f_[j][4] = f_star[j][2];
		lat.f_[j][8] = r*f_star[j][6] + (1-r)*f_star[j][5]+2*r*omega_i[8]*lat.rho_[j]*uw/(cs*cs)*xi[8][0];
		lat.f_[j][7] = r*f_star[j][5] + (1-r)*f_star[j][6]+2*r*omega_i[7]*lat.rho_[j]*uw/(cs*cs)*xi[7][0];
		break;
		case 8: //NE
		lat.f_[j][4] = f_star[j][2];
		lat.f_[j][8] = r*f_star[j][6] + (1-r)*f_star[j][5]+2*r*omega_i[8]*lat.rho_[j]*uw/(cs*cs)*xi[8][0];
		lat.f_[j][7] = r*f_star[j][5] + (1-r)*f_star[j][6]+2*r*omega_i[7]*lat.rho_[j]*uw/(cs*cs)*xi[7][0];
		break;
	}
}


//Fonction de traitement des conditions aux frontières : Diffuse bounce-back (bounce-back + maxwellian diffusion) sur la frontière NORD avec paroi mobile (pour un écoulement de Couette)
void DBB_N_BC_Couette(int j, int cas, Lattice lat, double beta, double** f_star,double cs, double* Uw, double* buffer, double* omega_i,double** xi, int D, int Q, double*** Qi, double sigma)
{
	double* buffer2 = new double[Q];
	/*for (int k=0;k<Q;k++)
	{
		fi_equilibre_v2 (j,k,lat.rho_[j],cs,lat,Uw,xi,D,Qi,buffer,omega_i,sigma,buffer2);
	}*/
	simplified_fi_equilibre (j,lat.rho_[j],cs,lat,Uw, xi,D,omega_i, buffer2,Q); //Calcul de fi,eq au mur (densité du mur, vitesse du mur)

	switch(cas)
	{	
		case 3: //N
		lat.f_[j][4] =  beta* f_star[j][2] + (1-beta)*(f_star[j][2]+f_star[j][5]+f_star[j][6])/(buffer2[2]+buffer2[5]+buffer2[6])*buffer2[4];
		lat.f_[j][8] =  beta* f_star[j][6] + (1-beta)*(f_star[j][2]+f_star[j][5]+f_star[j][6])/(buffer2[2]+buffer2[5]+buffer2[6])*buffer2[8];
		lat.f_[j][7] =  beta* f_star[j][5] + (1-beta)*(f_star[j][2]+f_star[j][5]+f_star[j][6])/(buffer2[2]+buffer2[5]+buffer2[6])*buffer2[7];
		break;
		case 7: //NO
		lat.f_[j][4] =  beta* f_star[j][2] + (1-beta)*(f_star[j][2]+f_star[j][5]+f_star[j][6])/(buffer2[2]+buffer2[5]+buffer2[6])*buffer2[4];
		lat.f_[j][8] =  beta* f_star[j][6] + (1-beta)*(f_star[j][2]+f_star[j][5]+f_star[j][6])/(buffer2[2]+buffer2[5]+buffer2[6])*buffer2[8];
		lat.f_[j][7] =  beta* f_star[j][5] + (1-beta)*(f_star[j][2]+f_star[j][5]+f_star[j][6])/(buffer2[2]+buffer2[5]+buffer2[6])*buffer2[7];
		break;
		case 8: //NE
		lat.f_[j][4] =  beta* f_star[j][2] + (1-beta)*(f_star[j][2]+f_star[j][5]+f_star[j][6])/(buffer2[2]+buffer2[5]+buffer2[6])*buffer2[4];
		lat.f_[j][8] =  beta* f_star[j][6] + (1-beta)*(f_star[j][2]+f_star[j][5]+f_star[j][6])/(buffer2[2]+buffer2[5]+buffer2[6])*buffer2[8];
		lat.f_[j][7] =  beta* f_star[j][5] + (1-beta)*(f_star[j][2]+f_star[j][5]+f_star[j][6])/(buffer2[2]+buffer2[5]+buffer2[6])*buffer2[7];
		break;
	}
	delete[] buffer2;
}

//Fonction de traitement des conditions aux frontières : Diffuse bounce-back (bounce-back + maxwellian diffusion) sur la frontière NORD
void DBB_N_BC(int j, int cas, Lattice lat, double beta, double** f_star)
{
	switch(cas)
	{	
		case 3: //N
		lat.f_[j][4] = beta* f_star[j][2] + (1-beta)*2./3.*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		lat.f_[j][8] = beta* f_star[j][6] + (1-beta)*1./6.*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		lat.f_[j][7] = beta* f_star[j][5] + (1-beta)*1./6.*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		break;
		case 7: //NO
		lat.f_[j][4] = beta* f_star[j][2] + (1-beta)*2./3.*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		lat.f_[j][8] = beta* f_star[j][6] + (1-beta)*1./6.*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		lat.f_[j][7] = beta* f_star[j][5] + (1-beta)*1./6.*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		break;
		case 8: //NE
		lat.f_[j][4] = beta* f_star[j][2] + (1-beta)*2./3.*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		lat.f_[j][8] = beta* f_star[j][6] + (1-beta)*1./6.*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		lat.f_[j][7] = beta* f_star[j][5] + (1-beta)*1./6.*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		break;
	}
}

//Fonction de traitement des conditions aux frontières : Diffuse bounce-back (bounce-back + maxwellian diffusion) sur la frontière SUD
void DBB_S_BC(int j, int cas, Lattice lat, double beta, double** f_star)
{
		switch (cas)
	{
		case 4: //S
		lat.f_[j][2] = beta* f_star[j][4] + (1-beta)*2./3.*(f_star[j][4] + f_star[j][7] + f_star[j][8]);
		lat.f_[j][5] = beta* f_star[j][7] + (1-beta)*1./6.*(f_star[j][4] + f_star[j][7] + f_star[j][8]);
		lat.f_[j][6] = beta* f_star[j][8] + (1-beta)*1./6.*(f_star[j][4] + f_star[j][7] + f_star[j][8]);
		break;
		case 5: //SO
		lat.f_[j][2] = beta* f_star[j][4] + (1-beta)*2./3.*(f_star[j][4] + f_star[j][7] + f_star[j][8]);
		lat.f_[j][5] = beta * f_star[j][7] + (1-beta)*1./6.*(f_star[j][4] + f_star[j][7] + f_star[j][8]);
		lat.f_[j][6] = beta* f_star[j][8] + (1-beta)*1./6.*(f_star[j][4] + f_star[j][7] + f_star[j][8]);
		break;
		case 6: //SE
		lat.f_[j][2] = beta* f_star[j][4] + (1-beta)*2./3.*(f_star[j][4] + f_star[j][7] + f_star[j][8]);
		lat.f_[j][5] = beta* f_star[j][7] + (1-beta)*1./6.*(f_star[j][4] + f_star[j][7] + f_star[j][8]);
		lat.f_[j][6] = beta* f_star[j][8] + (1-beta)*1./6.*(f_star[j][4] + f_star[j][7] + f_star[j][8]);
		break;
	}
}

//Fonction de traitement des conditions aux frontières : Multi-relection (Specular reflection + maxwellian diffusion) sur la frontière NORD
void MR_N_BC(int j, int cas, Lattice lat, double sigma, double** f_star)
{
	switch(cas)
	{
		case 3: //N
		lat.f_[j][4] = (1-sigma) * f_star[j][2] + (sigma)*2/3*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		lat.f_[j][8] = (1-sigma) * f_star[j][5] + (sigma)*1/6*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		lat.f_[j][7] = (1-sigma) * f_star[j][6] + (sigma)*1/6*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		break;
		case 7: //NO
		lat.f_[j][4] = (1-sigma) * f_star[j][2] + (sigma)*2/3*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		lat.f_[j][8] = (1-sigma) * f_star[j][5] + (sigma)*1/6*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		lat.f_[j][7] = (1-sigma) * f_star[j][6] + (sigma)*1/6*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		break;
		case 8: //NE
		lat.f_[j][4] = (1-sigma) * f_star[j][2] + (sigma)*2/3*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		lat.f_[j][8] = (1-sigma) * f_star[j][5] + (sigma)*1/6*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		lat.f_[j][7] = (1-sigma) * f_star[j][6] + (sigma)*1/6*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		break;
	}
}

//Fonction de traitement des conditions aux frontières : Multi-relection (Specular reflection + maxwellian diffusion) sur la frontière SUD
void MR_S_BC(int j, int cas, Lattice lat, double sigma, double** f_star)
{
	switch (cas)
	{
		case 4: //S
		lat.f_[j][2] = (1-sigma) * f_star[j][4] + sigma*2/3*(f_star[j][4] + f_star[j][5] + f_star[j][6]);
		lat.f_[j][5] = (1-sigma) * f_star[j][8] + sigma*1/6*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		lat.f_[j][6] = (1-sigma) * f_star[j][7] + sigma*1/6*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		break;
		case 5: //SO
		lat.f_[j][2] = (1-sigma) * f_star[j][4] + sigma*2/3*(f_star[j][4] + f_star[j][5] + f_star[j][6]);
		lat.f_[j][5] = (1-sigma) * f_star[j][8] + sigma*1/6*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		lat.f_[j][6] = (1-sigma) * f_star[j][7] + sigma*1/6*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		break;
		case 6: //SE
		lat.f_[j][2] = (1-sigma) * f_star[j][4] + sigma*2/3*(f_star[j][4] + f_star[j][5] + f_star[j][6]);
		lat.f_[j][5] = (1-sigma) * f_star[j][8] + sigma*1/6*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		lat.f_[j][6] = (1-sigma) * f_star[j][7] + sigma*1/6*(f_star[j][2] + f_star[j][5] + f_star[j][6]);
		break;
	}
}

//Fonction de traitement des conditions d'entrée/sortie : Vitesse imposée en entrée, condition régularisée de Latt
void regularized_BC_v_inlet(int j,int k,Lattice lat,double cs,double** v_in, int nx,double** xi,int D,double*** Qi, double* buffer, double* omega_i,int cas,double** Pi_neq,int Q, double** f_neq, int* bb, double sigma)
{
	if (cas ==1 || cas ==5 || cas==7)
	{	
		lat.rho_[j] =  1/(1-v_in[j/nx][0])*(2*(lat.f_[j][3]+lat.f_[j][6]+lat.f_[j][7])+lat.f_[j][0] + lat.f_[j][2]+lat.f_[j][4]);
		for (k=0;k<Q;k++)
		{
			fi_equilibre (j,k,lat.rho_[j],cs,lat,v_in[j/nx],xi,D,Qi, buffer, omega_i,sigma);
			f_neq[j][k] = lat.f_[j][k] - lat.f0_[j][k];
		}				
		lat.f_[j][1] = lat.f0_[j][1] + f_neq[j][3];
		lat.f_[j][5] = lat.f0_[j][5] + f_neq[j][7];
		lat.f_[j][8] = lat.f0_[j][8] + f_neq[j][6];
		//Calcul de Pi (neq)
		PI_neq_inlet(Pi_neq,j,Q,lat,Qi,buffer,bb);
		fi_bar(omega_i,Qi,Pi_neq,cs,j,D,lat,buffer,Q);
	}
}

//Fonction de traitement des conditions d'entrée/sortie : Pression imposée en entrée, condition régularisée de Latt
void regularized_BC_p_inlet(int j,int k,Lattice lat,double cs,double rho_in,double** xi,int D,double*** Qi, double* buffer, double* omega_i,int cas,double** Pi_neq,int Q, double** f_neq, int* bb ,double sigma)
{
	if (cas ==1 || cas ==5 || cas==7)
	{	
		lat.u_[j][0] = 1-(2*(lat.f_[j][3]+lat.f_[j][6]+lat.f_[j][7])+lat.f_[j][0] + lat.f_[j][2]+lat.f_[j][4])/rho_in;
		lat.u_[j][1] = 0;
		for (k=0;k<Q;k++)
		{
			fi_equilibre (j,k,rho_in,cs,lat,lat.u_[j],xi,D,Qi, buffer, omega_i,sigma);
			f_neq[j][k] = lat.f_[j][k] - lat.f0_[j][k];
		}				
		lat.f_[j][1] = lat.f0_[j][1] + f_neq[j][3];
		lat.f_[j][5] = lat.f0_[j][5] + f_neq[j][7];
		lat.f_[j][8] = lat.f0_[j][8] + f_neq[j][6];
		//Calcul de Pi (neq)
		PI_neq_inlet(Pi_neq,j,Q,lat,Qi,buffer,bb);
		fi_bar(omega_i,Qi,Pi_neq,cs,j,D,lat,buffer,Q);
	}
}

//Fonction de traitement des conditions d'entrée/sortie : Vitesse imposée en sortie, condition régularisée de Latt
void regularized_BC_v_outlet(int j,int k,Lattice lat,double cs,double** v_out, int nx,double** xi,int D,double*** Qi, double* buffer, double* omega_i,int cas,double** Pi_neq,int Q, double** f_neq, int* bb, double sigma)
{
	lat.rho_[j] = 1./(1+v_out[j/nx][0])*(2*(lat.f_[j][1]+lat.f_[j][5]+lat.f_[j][8])+lat.f_[j][0] + lat.f_[j][2]+lat.f_[j][4]);
	if (cas ==2 || cas ==6 || cas==8)
	{
		for (k=0;k<Q;k++)
		{
			//Calcul de fi (eq) avec les conditions de sortie (pression imposée)
			fi_equilibre (j,k,lat.rho_[j],cs,lat,v_out[j/nx],xi,D,Qi, buffer, omega_i,sigma);
			//Calcul de des fi (neq) = fi - fi(eq) et Pi(neq) = sum (Qi * fi(neq))	
			f_neq[j][k] = lat.f_[j][k] - lat.f0_[j][k];
		}
		lat.f_[j][3] = lat.f0_[j][3] + f_neq[j][1];
		lat.f_[j][6] = lat.f0_[j][6] + f_neq[j][8];
		lat.f_[j][7] = lat.f0_[j][7] + f_neq[j][5];
		//Calcul de Pi (neq)
		PI_neq_outlet(Pi_neq,j,Q,lat,Qi,buffer,bb);
		fi_bar(omega_i,Qi,Pi_neq,cs,j,D,lat,buffer,Q);
	}
}

//Fonction de traitement des conditions d'entrée/sortie : Pression imposée en sortie, condition régularisée de Latt
void regularized_BC_p_outlet(int j,int k,Lattice lat,double cs,double rho_out,double** xi,int D,double*** Qi, double* buffer, double* omega_i,int cas,double** Pi_neq,int Q, double** f_neq, int* bb, double sigma)
{
	if (cas ==2 || cas ==6 || cas==8)
	{
		lat.u_[j][0] = -1+(2*(lat.f_[j][1]+lat.f_[j][5]+lat.f_[j][8])+lat.f_[j][0] + lat.f_[j][2]+lat.f_[j][4])/rho_out;
		lat.u_[j][1] = 0;
		for (k=0;k<Q;k++)
		{
			//Calcul de fi (eq) avec les conditions de sortie (pression imposée)
			fi_equilibre (j,k,rho_out,cs,lat,lat.u_[j],xi,D,Qi, buffer, omega_i,sigma);
			//Calcul de des fi (neq) = fi - fi(eq) et Pi(neq) = sum (Qi * fi(neq))	
			f_neq[j][k] = lat.f_[j][k] - lat.f0_[j][k];
		}
		lat.f_[j][3] = lat.f0_[j][3] + f_neq[j][1];
		lat.f_[j][6] = lat.f0_[j][6] + f_neq[j][8];
		lat.f_[j][7] = lat.f0_[j][7] + f_neq[j][5];
		//Calcul de Pi (neq)
		PI_neq_outlet(Pi_neq,j,Q,lat,Qi,buffer,bb);
		fi_bar(omega_i,Qi,Pi_neq,cs,j,D,lat,buffer,Q);
	}
}

