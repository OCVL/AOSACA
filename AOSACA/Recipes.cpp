// Recipes.cpp: implementation of the CRecipes class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AOSACA.h"
#include "OptCalc.h"
#include "Recipes.h"
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRecipes::CRecipes()
{

}

//////////////////////////////////////////////////////////////////////
CRecipes::~CRecipes()
{

}

//////////////////////////////////////////////////////////////////////
float *CRecipes::vector(long nl, long nh)
{
	float *v;
    v=(float *)malloc((size_t)((nh-nl+1+NR_END)*sizeof(float)));
    if(!v) nrerror("allocation failure in vector()");
	return v-nl+NR_END;
}


//////////////////////////////////////////////////////////////////////
void CRecipes::nrerror(char error_text[])
{
	wchar_t szBuff[64];
	swprintf(szBuff, 64, L"%p", error_text);	
    MessageBox(NULL,szBuff,_T("Error"),MB_OK);
//	exit(1);
}


//////////////////////////////////////////////////////////////////////
void CRecipes::free_vector(float *v,long nl, long nh)
{
	free((FREE_ARG)(v+nl-NR_END));
}

//////////////////////////////////////////////////////////////////////
BOOL CRecipes::ludcmp(float a[MAX_TERM_NUMBER+1][MAX_TERM_NUMBER+1],int n,int *indx, float *d)
{
	int i,imax,j,k;
	float big,dum,sum,temp;
	float *vv;

	vv=vector(1,n);
	*d=1.0;
	for(i=1;i<=n;i++)
	{
		big=0.0;
		for(j=1;j<=n;j++)
		{
			temp=(float)(fabs(a[i][j]));
		    if(temp>big) big=temp;
		}
		if(big==0.0) 
		{
		//	nrerror("Singular matrix in routine ludcmp");			
				return FALSE;
		}
		vv[i]=(float)(1.0/big);
	}
	for(j=1;j<=n;j++)
	{
		for(i=1;i<j;i++)
		{
			sum=a[i][j];
			for(k=1;k<i;k++)
			   sum-=a[i][k]*a[k][j];
            a[i][j]=sum;
		}
		big=0.0;
		for(i=j;i<=n;i++)
		{
			sum=a[i][j];
			for(k=1;k<j;k++)
			    sum-=a[i][k]*a[k][j];
			a[i][j]=sum;
			dum=(float)(vv[i]*fabs(sum));
			if(dum>=big)
			{
				big=dum;
				imax=i;
			}
		}
		if(j!=imax)
		{
			for(k=1;k<=n;k++)
			{
				dum=a[imax][k];
				a[imax][k]=a[j][k];
				a[j][k]=dum;
			}
			*d=-(*d);
			vv[imax]=vv[j];
		}
		indx[j]=imax;
		if(a[j][j]==0.0) a[j][j]=(float)TINY;
		if(j!=n)
		{
			dum=(float)(1.0/(a[j][j]));
			for(i=j+1;i<=n;i++)   a[i][j]*=dum;
		}
	}
	free_vector(vv,1,n);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
void CRecipes::lubksb(float a[MAX_TERM_NUMBER+1][MAX_TERM_NUMBER+1],
					  int n, int *indx,float b[])
{
	int i,ii=0,ip,j;
	float sum;

	for(i=1;i<=n;i++)
	{
		ip=indx[i];
		sum=b[ip];
		b[ip]=b[i];
		if(ii)
			for(j=ii;j<=i-1;j++)  sum-=a[i][j]*b[j];///a bug 
			else if (sum) ii=i;
			b[i]=sum;
	}
	for(i=n;i>=1;i--)
	{
		sum=b[i];
		for(j=i+1;j<=n;j++)  sum-=a[i][j]*b[j];
		b[i]=sum/a[i][i];
	}
}

//////////////////////////////////////////////////////////////////////
/*************************************************************************
 *
 * FAST FFT in L dimensions (L >= 2).
 *
 * H(n1, ..., nL) == Sum_{kL: 0 to NL-1} ... Sum_{k1: 0 to N1-1}
 *      h(k1, ..., kL) * exp(2*PI*i*kL*nL/NL) * ... * exp(2*PI*i*k1*n1/N1)
 *
 * where n1 and k1 range from 0 to N1 - 1, ...,
 *       nL and kL range from 0 to NL - 1, ...
 *
 * The inverse transform:
 *    From the above formula, change the i's in the exponentials to -i's,
 *    and put an overall factor of 1/(N1 * ... * nL) in front of the whole
 *    thing.
 *
 * Note: the notion "i" mentioned above means sqrt(-1).
 *
 * **********************************************************************
 *
 * Fourier_Transform(float data[], unsigned long nn[], int ndim, int isign):
 *
 * isign means the sign of "i" in the above formula.
 *
 * If isign is input as 1, replaces data by its ndim-dimensional discrete
 *      Fourier transform.
 *    nn[1..ndim] is an integer array containing the lengths of each
 *      dimension (number of complex values), which MUST all be powers of 2.
 *    data[] is a real array of length twice the product of these lengths,
 *      in which the data are stored as in a multidimensional complex array:
 *      real and imaginary parts of each element are in consecutive locations,
 *      and the rightmost index of the array increases most rapidly as one
 *      proceeds along data.
 *    For a two-dimensional array, this is equivalent to storing the array
 *      by rows.
 *
 * If isign is input as -1, data is replaced by its inverse transform times
 *    the product of the lengths of all dimensions.
 *
 **************************************************************************/
void CRecipes::Fourier_Transform(float data[], unsigned long nn[], int ndim, int isign)
{
	int idim;
	unsigned long i1, i2, i3, i2rev, i3rev, ip1, ip2, ip3, ifp1, ifp2;
	unsigned long ibit, k1, k2, n, nprev, nrem, ntot;
	float tempi, tempr;
	double theta, wi, wpi, wpr, wr, wtemp; // double precision for trigonometric recurrences

	// compute total number of complex values
	for (ntot = 1, idim = 1; idim <= ndim; idim++)
		ntot *= nn[idim];

	// main loop over the dimensions
	nprev = 1;
	for (idim = ndim; idim >= 1; idim--) {
		n = nn[idim];
		nrem = ntot / (n * nprev);
		ip1 = nprev << 1;
		ip2 = ip1 * n;
		ip3 = ip2 * nrem;
		i2rev = 1;
		// this is the bit-reversal section of the routine
		for (i2 = 1; i2 <= ip2; i2 += ip1) {
			if (i2 < i2rev) {
				for (i1 = i2; i1 <= i2 + ip1 - 2; i1 += 2) {
					for (i3 = i1; i3 <= ip3; i3 += ip2) {
						i3rev = i2rev + i3 - i2;
						//SWAP(data[i3], data[i3rev]);
						tempr = data[i3];
						data[i3] = data[i3rev];
						data[i3rev] = tempr;
						//SWAP(data[i3 + 1], data[i3rev + 1]);
						tempr = data[i3 + 1];
						data[i3 + 1] = data[i3rev + 1];
						data[i3rev + 1] = tempr;
					}
				}
			}
			ibit = ip2 >> 1;
			while (ibit >= ip1 && i2rev > ibit) {
				i2rev -= ibit;
				ibit >>= 1;
			}
			i2rev += ibit;
		}
		
		// here begins the Danielson-Lanczos section of the routine
		ifp1 = ip1;		
		while (ifp1 < ip2) {
			ifp2 = ifp1 << 1;
			theta = isign * 6.28318530717959 / (ifp2 / ip1); // initialize for the trig. recurrence.
			wtemp = sin(0.5 * theta);
			wpr = -2.0 * wtemp * wtemp;
			wpi = sin(theta);
			wr = 1.0;
			wi = 0.0;
			for (i3 = 1; i3 < ifp1; i3 += ip1) {
				for (i1 = i3; i1 <= i3 + ip1 - 2; i1 += 2) {
					for (i2 = i1; i2 <= ip3; i2 += ifp2) {
						k1 = i2; // Danielson-Lanczos formula
						k2 = k1 + ifp1;
						tempr = (float) wr * data[k2] - (float) wi * data[k2 + 1];
						tempi = (float) wr * data[k2 + 1] + (float) wi * data[k2];
						data[k2] = data[k1] - tempr;
						data[k2 + 1] = data[k1 + 1] - tempi;
						data[k1] += tempr;
						data[k1 + 1] += tempi;
					}
				}
				wr = (wtemp = wr) * wpr - wi * wpi + wr; // trigonometric recurrence
				wi = wi * wpr + wtemp * wpi + wi;
			}
			ifp1 = ifp2;
		}
		nprev *= n;
	}
}
//////////////////////////////////////////////////////////////////////
/*
 * FFT2 performs 2-dimentional discrete Fourier transform.
 *
 * Input data is a complex N by N matrix, with real part xr, and imaginary part xi.
 * Output data is a complex N by N matrix, with real part XR and imaginary part XI.
 *
 * Allow xi to be NULL.
 */
void CRecipes::CalcFFT2(double *XR, double *XI, 
						const double *xr, const double *xi, unsigned long N)
{
	unsigned long row, col, pos;
	unsigned long nn[3];
	float *data;

	data = new float [2 * N * N + 1];
	pos = 0;
	for (row = 0; row < N; row++) {
		for (col = 0; col < N; col++) {
			//pos = row * N + col;
			data[2 * pos + 1] = (float) xr[pos];
			if (xi == NULL)
				data[2 * pos + 2] = 0.0;
			else
				data[2 * pos + 2] = (float) xi[pos];
			pos++;
		}
	}
	
	nn[1] = N; nn[2] = N;
	Fourier_Transform(data, nn, 2, -1);// nn[0] and data[0] would not be used.

	
	for (row = 0; row < N; row++) {
		for (col = 0; col < N; col++) {
			pos = row * N + col;
			XR[pos] = data[2 * pos + 1];
			XI[pos] = data[2 * pos + 2];
		}
	}
	delete [] data;
}
//////////////////////////////////////////////////////////////////////
/*************************************************************************
 * For N x N matrix, let A be the left-top p x p submatrix, p = N / 2:
 *
 *            A B
 *            C D
 *
 * then after fftshift, the new matrix is:
 *
 *            D C
 *            B A
 *
 **************************************************************************/
void CRecipes::ShiftFFT(double *X, const double *x, unsigned long N)
{
	unsigned long i, j, p;
	
	p = N / 2;
	
	for (i = 0; i < p; i++) 
	{
		for (j = 0; j < p; j++)
			X[(i + N - p) * N + (j + N - p)] = x[i * N + j];
		
		for (j = p; j < N; j++)
			X[(i + p) * N + (j - p)] = x[i * N + j];
	}
	
	for (i = p; i < N; i++) {
		for (j = 0; j < p; j++)
			X[(i - p) * N + (j + p)] = x[i * N + j];
		
		for (j = p; j < N; j++)
			X[(i - p) * N + (j - p)] = x[i * N + j];
	}
}
//////////////////////////////////////////////////////////////////////
