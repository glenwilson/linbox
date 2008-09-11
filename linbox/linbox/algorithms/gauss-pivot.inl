// =================================================================== //
// SparseElimination search for pivots 
// Time-stamp: <11 Sep 08 13:44:32 Jean-Guillaume.Dumas@imag.fr> 
// =================================================================== //
#ifndef __GAUSS_PIVOT_INL
#define __GAUSS_PIVOT_INL

namespace LinBox 
{

    template <class _Field>
    template <class Vector, class D> void 
    GaussDomain<_Field>::SparseFindPivot (Vector        	&lignepivot,
                                          unsigned long 	&indcol,
                                          long 		&indpermut,
                                          D             	&columns,
                                          Element		&determinant)
    {
 
//        std::cerr << "SFP BEG : lignepivot: [";
//         for(typename Vector::const_iterator refs =  lignepivot.begin();
//             refs != lignepivot.end() ;
//             ++refs )
//             std::cerr << '(' << refs->first << ';' << refs->second << ')';
//         std::cerr << "]" << std::endl;
	typedef typename Vector::value_type E;    

	long nj =  lignepivot.size ();

        bool pivoting = false;

	if (nj > 0) {
            indpermut = lignepivot[0].first;

            long ds = --columns[indpermut], dl, p = 0;

            for (long j = 1; j < nj; ++j) {
                if ((dl = --columns[lignepivot[j].first]) < ds) {
                    ds = dl;
                    p = j;
                }
            }

            if (p != 0) {
                pivoting = true;
                if (indpermut == static_cast<long>(indcol)) {
                    indpermut = lignepivot[p].first;
                    std::swap( lignepivot[p].second, lignepivot[0].second);
                } else {
                    E ttm = lignepivot[p];
                    indpermut = ttm.first;

                    for (long m = p; m; --m)
                        lignepivot[m] = lignepivot[m-1];

                    lignepivot[0] = ttm;
                }
            }

            _F.mulin(determinant, lignepivot[0].second);
            if (indpermut != static_cast<long>(indcol)) {
// std::cerr << "Permuting col: " << indpermut << " <--> " << indcol << std::endl;
                    // no need to decrement/increment, already done during the search
                lignepivot[0].first = indcol;
                pivoting = true;
            }

            if (pivoting) _F.negin(determinant);
            ++indcol;
	} else
            indpermut = -1;

            //        std::cerr << "SFP END : lignepivot: [";
//         for(typename Vector::const_iterator refs =  lignepivot.begin();
//             refs != lignepivot.end() ;
//             ++refs )
//             std::cerr << '(' << refs->first << ';' << refs->second << ')';
//         std::cerr << "]" << std::endl;
    }


    template <class _Field>
    template <class Vector> void 
    GaussDomain<_Field>::SparseFindPivot (Vector &lignepivot, 
                                          unsigned long &indcol, 
                                          long &indpermut, 
                                          Element& determinant)
    {
	long nj = lignepivot.size ();

	if (nj > 0) {
            indpermut = lignepivot[0].first;
            _F.mulin(determinant, lignepivot[0].second);
            if (indpermut != static_cast<long>(indcol)){
// std::cerr << "Permuting col: " << lignepivot[0].first << " <--> " << indcol << std::endl;
                lignepivot[0].first = indcol;
                _F.negin(determinant);
            }
            ++indcol;
	} else
            indpermut = -1;
    }

    template <class _Field>
    template <class Vector> void 
    GaussDomain<_Field>::FindPivot (Vector &lignepivot, 
                                    unsigned long &k, 
                                    long &indpermut)
    { 
            // Dense lignepivot
	long n = lignepivot.size ();
	long j = k;

	for (; j < n ; ++j )
            if (!_F.isZero (lignepivot[j])) break ;

	if (j == n )
            indpermut = -1 ;
	else {
            indpermut = j ;
            if (indpermut != k) {
                typename Vector::value_type tmp = lignepivot[k] ;
                lignepivot[k] = lignepivot[j] ;
                lignepivot[j] = tmp ;
            }

            ++k;
	}
    }


} // namespace LinBox

#endif // __GAUSS_INL
