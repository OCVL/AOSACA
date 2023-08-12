// Recipes.h: interface for the CRecipes class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RECIPES_H__6C299A7E_79BE_4DE1_9F93_FFB73649B971__INCLUDED_)
#define AFX_RECIPES_H__6C299A7E_79BE_4DE1_9F93_FFB73649B971__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define NR_END 1
#define FREE_ARG char*

class CRecipes  
{
public:
	static float *CRecipes::vector(long nl, long nh);
	static void CRecipes::nrerror(char error_text[]);
	static void CRecipes::free_vector(float *v,long nl, long nh);
	static BOOL CRecipes::ludcmp(float a[MAX_TERM_NUMBER+1][MAX_TERM_NUMBER+1],
		int n,int *indx, float *d);
	static void CRecipes::lubksb(float a[MAX_TERM_NUMBER+1][MAX_TERM_NUMBER+1],
					  int n, int *indx,float b[]);
	static void CRecipes::Fourier_Transform(float data[], 
		unsigned long nn[], int ndim, int isign);
	static void CRecipes::CalcFFT2(double *XR, double *XI, 
						const double *xr, const double *xi, unsigned long N);
	static void CRecipes::ShiftFFT(double *X, const double *x, unsigned long N);

	CRecipes();
	virtual ~CRecipes();

};

#endif // !defined(AFX_RECIPES_H__6C299A7E_79BE_4DE1_9F93_FFB73649B971__INCLUDED_)
