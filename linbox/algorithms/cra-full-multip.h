/* linbox/algorithms/cra-full-multip.h
 * Copyright (C) 1999-2010 The LinBox group
 *
 * Time-stamp: <15 Dec 10 15:54:00 Jean-Guillaume.Dumas@imag.fr>
 *
 * ========LICENCE========
 * This file is part of the library LinBox.
 *
  * LinBox is free software: you can redistribute it and/or modify
 * it under the terms of the  GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ========LICENCE========
 */

/*!@file algorithms/cra-full-multip.h
 * @ingroup algorithms
 * @brief NO DOC
 */

#ifndef __LINBOX_cra_full_multip_H
#define __LINBOX_cra_full_multip_H

#include "linbox/util/timer.h"
#include <stdlib.h>
#include "linbox/integer.h"
#include "linbox/solutions/methods.h"
#include "linbox/vector/blas-vector.h"
#include <utility>

#include "linbox/algorithms/lazy-product.h"

namespace LinBox
{

	/*! NO DOC...
	 * @ingroup CRA
	 * @bib
	 * - Jean-Guillaume Dumas, Thierry Gautier et Jean-Louis Roch.  <i>Generic design
	 * of Chinese remaindering schemes</i>  PASCO 2010, pp 26-34, 21-23 juillet,
	 * Grenoble, France.
	 */
	template<class Domain_Type>
	struct FullMultipCRA {
		typedef Domain_Type			Domain;
		typedef typename Domain::Element DomainElement;
		typedef FullMultipCRA<Domain> 		Self_t;

	protected:
		std::vector< double >           	RadixSizes_;
		std::vector< LazyProduct >      	RadixPrimeProd_;
		std::vector< BlasVector<Givaro::ZRing<Integer> > >  RadixResidues_;
		std::vector< bool >             	RadixOccupancy_;
		const double				LOGARITHMIC_UPPER_BOUND;
		double					totalsize;

	public:
		// LOGARITHMIC_UPPER_BOUND is the natural logarithm
		// of an upper bound on the resulting integers
		FullMultipCRA(const double b=0.0) :
			LOGARITHMIC_UPPER_BOUND(b), totalsize(0.0)
		{}

		Integer& getModulus(Integer& m)
		{
			Givaro::ZRing<Integer> ZZ;
			BlasVector<Givaro::ZRing<Integer> > r(ZZ); result(r);
			return m=RadixPrimeProd_.back()();
		}

		template<template<class> class Vect>
		Vect<Integer>& getResidue(Vect<Integer>& r)
		{
			result(r);
			return r;
		}

		//! init
		template<class Vect>
		void initialize (const Integer& D, const Vect& e)
		{
			RadixSizes_.resize(1);
			RadixPrimeProd_.resize(1);
			Givaro::ZRing<Integer> ZZ ;
			const BlasVector<Givaro::ZRing<Integer> >z(ZZ);
			RadixResidues_.resize(1,z);
			RadixOccupancy_.resize(1); RadixOccupancy_.front() = false;
			progress( D, e);
#if 0
			std::vector< double >::iterator  _dsz_it = RadixSizes_.begin();
			std::vector< LazyProduct >::iterator _mod_it = RadixPrimeProd_.begin();
			std::vector< BlasVector<Givaro::ZRing<Integer> > >::iterator _tab_it = RadixResidues_.begin();
			std::vector< bool >::iterator    _occ_it = RadixOccupancy_.begin();
			_mod_it->initialize(D);
			*_dsz_it =  Givaro::naturallog(D);

			typename Vect::const_iterator e_it = e.begin();
			_tab_it->resize(e.size());
			BlasVector<Givaro::ZRing<Integer> >::iterator t0_it= _tab_it->begin();
			for( ; e_it != e.end(); ++e_it, ++ t0_it)
				*t0_it = *e_it;
			*_occ_it = true;
#endif
			return;
		}

		template<class Vect>
		void initialize (const Domain& D, const Vect& e)
		{
			RadixSizes_.resize(1);
			RadixPrimeProd_.resize(1);
			Givaro::ZRing<Integer> ZZ;
			RadixResidues_.resize(1,BlasVector<Givaro::ZRing<Integer> >(ZZ));
			RadixOccupancy_.resize(1); RadixOccupancy_.front() = false;
			progress(D, e);
		}

		template<class OKDomain>
		void initialize (const Domain& D, const BlasVector<OKDomain>& e)
		{
			RadixSizes_.resize(1);
			RadixPrimeProd_.resize(1);
			Givaro::ZRing<Integer> ZZ ;
			RadixResidues_.resize(1,BlasVector<Givaro::ZRing<Integer> >(ZZ));
			RadixOccupancy_.resize(1); RadixOccupancy_.front() = false;
			progress(D, e);
		}

		//! progress
		/* Used in the case where D is a big Integer and Domain cannot be constructed */
		// template<template<class T> class Vect>
		template<class Vect>
		void progress (const Integer& D, const Vect& e)
		{
			std::vector< double >::iterator  _dsz_it = RadixSizes_.begin();
			std::vector< LazyProduct >::iterator _mod_it = RadixPrimeProd_.begin();
			std::vector< BlasVector<Givaro::ZRing<Integer> > >::iterator _tab_it = RadixResidues_.begin();
			std::vector< bool >::iterator    _occ_it = RadixOccupancy_.begin();
			BlasVector<Givaro::ZRing<Integer> > ri(e.field(),e.size());
			LazyProduct mi;
			double di;
			if (*_occ_it) {
				typename Vect::const_iterator  e_it = e.begin();
				BlasVector<Givaro::ZRing<Integer> >::iterator       ri_it = ri.begin();
				BlasVector<Givaro::ZRing<Integer> >::const_iterator t0_it = _tab_it->begin();
				Integer invprod; precomputeInvProd(invprod, D, _mod_it->operator()());
				for( ; e_it != e.end(); ++e_it, ++ri_it, ++ t0_it) {
					*ri_it =* e_it;
					smallbigreconstruct(*ri_it,  *t0_it, invprod );
				}
				Integer tmp = D;
				di = *_dsz_it + Givaro::naturallog(tmp);
				mi.mulin(tmp);
				mi.mulin(*_mod_it);
				*_occ_it = false;
			}
			else {
				Integer tmp = D;
				_mod_it->initialize(tmp);
				*_dsz_it = Givaro::naturallog(tmp);
				typename Vect::const_iterator e_it = e.begin();
				_tab_it->resize(e.size());
				BlasVector<Givaro::ZRing<Integer> >::iterator t0_it= _tab_it->begin();
				for( ; e_it != e.end(); ++e_it, ++ t0_it)
					*t0_it = *e_it;
				*_occ_it = true;
				return;
			}
			for(++_dsz_it, ++_mod_it, ++_tab_it, ++_occ_it ; _occ_it != RadixOccupancy_.end() ; ++_dsz_it, ++_mod_it, ++_tab_it, ++_occ_it) {
				if (*_occ_it) {
					BlasVector<Givaro::ZRing<Integer> >::iterator      ri_it = ri.begin();
					BlasVector<Givaro::ZRing<Integer> >::const_iterator t_it= _tab_it->begin();
					Integer invprod; precomputeInvProd(invprod, mi(), _mod_it->operator()());
					for( ; ri_it != ri.end(); ++ri_it, ++ t_it)
						smallbigreconstruct(*ri_it, *t_it, invprod);
					mi.mulin(*_mod_it);
					di += *_dsz_it;
					*_occ_it = false;
				}
				else {
					*_dsz_it = di;
					*_mod_it = mi;
					*_tab_it = ri;
					*_occ_it = true;
					return;
				}
			}

			RadixSizes_.push_back( di );
			RadixResidues_.push_back( ri );
			RadixPrimeProd_.push_back( mi );
			RadixOccupancy_.push_back ( true );
		}

		// spec for BlasVector
		void progress (const Integer& D, const BlasVector<Givaro::ZRing<Integer> >& e)
		{
			std::vector< double >::iterator  _dsz_it = RadixSizes_.begin();
			std::vector< LazyProduct >::iterator _mod_it = RadixPrimeProd_.begin();
			std::vector< BlasVector<Givaro::ZRing<Integer> > >::iterator _tab_it = RadixResidues_.begin();
			std::vector< bool >::iterator    _occ_it = RadixOccupancy_.begin();
			BlasVector<Givaro::ZRing<Integer> > ri(e.field(),e.size());
			LazyProduct mi; double di;
			if (*_occ_it) {
				typename BlasVector<Givaro::ZRing<Integer> >::const_iterator  e_it = e.begin();
				BlasVector<Givaro::ZRing<Integer> >::iterator       ri_it = ri.begin();
				BlasVector<Givaro::ZRing<Integer> >::const_iterator t0_it = _tab_it->begin();
				Integer invprod; precomputeInvProd(invprod, D, _mod_it->operator()());
				for( ; e_it != e.end(); ++e_it, ++ri_it, ++ t0_it) {
					*ri_it =* e_it;
					smallbigreconstruct(*ri_it,  *t0_it, invprod );
				}
				Integer tmp = D;
				di = *_dsz_it + Givaro::naturallog(tmp);
				mi.mulin(tmp);
				mi.mulin(*_mod_it);
				*_occ_it = false;
			}
			else {
				Integer tmp = D;
				_mod_it->initialize(tmp);
				*_dsz_it = Givaro::naturallog(tmp);
				typename BlasVector<Givaro::ZRing<Integer> >::const_iterator e_it = e.begin();
				_tab_it->resize(e.size());
				BlasVector<Givaro::ZRing<Integer> >::iterator t0_it= _tab_it->begin();
				for( ; e_it != e.end(); ++e_it, ++ t0_it)
					*t0_it = *e_it;
				*_occ_it = true;
				return;
			}
			for(++_dsz_it, ++_mod_it, ++_tab_it, ++_occ_it ; _occ_it != RadixOccupancy_.end() ; ++_dsz_it, ++_mod_it, ++_tab_it, ++_occ_it) {
				if (*_occ_it) {
					BlasVector<Givaro::ZRing<Integer> >::iterator      ri_it = ri.begin();
					BlasVector<Givaro::ZRing<Integer> >::const_iterator t_it= _tab_it->begin();
					Integer invprod; precomputeInvProd(invprod, mi(), _mod_it->operator()());
					for( ; ri_it != ri.end(); ++ri_it, ++ t_it)
						smallbigreconstruct(*ri_it, *t_it, invprod);
					mi.mulin(*_mod_it);
					di += *_dsz_it;
					*_occ_it = false;
				}
				else {
					*_dsz_it = di;
					*_mod_it = mi;
					*_tab_it = ri;
					*_occ_it = true;
					return;
				}
			}

			RadixSizes_.push_back( di );
			RadixResidues_.push_back( ri );
			RadixPrimeProd_.push_back( mi );
			RadixOccupancy_.push_back ( true );
		}

		template<class Vect>
		void progress (const Domain& D, const Vect& e)
		{
			// Radix shelves
			std::vector< double >::iterator  _dsz_it = RadixSizes_.begin();
			std::vector< LazyProduct >::iterator _mod_it = RadixPrimeProd_.begin();
			std::vector< BlasVector<Givaro::ZRing<Integer> > >::iterator _tab_it = RadixResidues_.begin();
			std::vector< bool >::iterator    _occ_it = RadixOccupancy_.begin();
			Givaro::ZRing<Integer> ZZ;
			BlasVector<Givaro::ZRing<Integer> > ri(ZZ,e.size());
			LazyProduct mi; double di;
			if (*_occ_it) {
				// If lower shelf is occupied
				// Combine it with the new residue
				// The for loop will try to put the resulting combination on the upper shelf
				typename Vect::const_iterator  e_it = e.begin();
				BlasVector<Givaro::ZRing<Integer> >::iterator       ri_it = ri.begin();
				BlasVector<Givaro::ZRing<Integer> >::const_iterator t0_it = _tab_it->begin();
				DomainElement invP0; precomputeInvP0(invP0, D, _mod_it->operator()() );
				for( ; ri_it != ri.end(); ++e_it, ++ri_it, ++ t0_it)
					fieldreconstruct(*ri_it, D, *e_it, *t0_it, invP0, (*_mod_it).operator()() );
				Integer tmp; D.characteristic(tmp);
				double ltp = Givaro::naturallog(tmp);
				di = *_dsz_it + ltp;
				totalsize += ltp;
				mi.mulin(tmp);
				mi.mulin(*_mod_it);
				*_occ_it = false;
			}
			else {
				// Lower shelf is free
				// Put the new residue here and exit
				Integer tmp; D.characteristic(tmp);
				double ltp =  Givaro::naturallog(tmp);
				_mod_it->initialize(tmp);
				*_dsz_it = ltp;
				totalsize += ltp;
				typename Vect::const_iterator e_it = e.begin();
				_tab_it->resize(e.size());
				BlasVector<Givaro::ZRing<Integer> >::iterator t0_it= _tab_it->begin();
				for( ; e_it != e.end(); ++e_it, ++ t0_it)
					D.convert(*t0_it, *e_it);
				*_occ_it = true;
				return;
			}

			// We have a combination to put in the upper shelf
			for(++_dsz_it, ++_mod_it, ++_tab_it, ++_occ_it ; _occ_it != RadixOccupancy_.end() ; ++_dsz_it, ++_mod_it, ++_tab_it, ++_occ_it) {
				if (*_occ_it) {
					// This shelf is occupied
					// Combine it with the new combination
					// The loop will try to put it on the upper shelf
					BlasVector<Givaro::ZRing<Integer> >::iterator      ri_it = ri.begin();
					BlasVector<Givaro::ZRing<Integer> >::const_iterator t_it= _tab_it->begin();

					Integer invprod; precomputeInvProd(invprod, mi(), _mod_it->operator()());
					for( ; ri_it != ri.end(); ++ri_it, ++ t_it)
						smallbigreconstruct(*ri_it, *t_it, invprod);

					// Product (lazy) computation
					mi.mulin(*_mod_it);

					// Moding out
					for(ri_it = ri.begin() ; ri_it != ri.end(); ++ri_it) {
						*ri_it %= mi();
					}

					di += *_dsz_it;
					*_occ_it = false;
				}
				else {
					// This shelf is free
					// Put the new combination here and exit
					*_dsz_it = di;
					*_mod_it = mi;
					*_tab_it = ri;
					*_occ_it = true;
					return;
				}
			}
			// All the shelfves were occupied
			// We create a new top shelf
			// And put the new combination there
			RadixSizes_.push_back( di );
			RadixResidues_.push_back( ri );
			RadixPrimeProd_.push_back( mi );
			RadixOccupancy_.push_back ( true );
		}

		// spec for BlasVector
		template<class OKDomain>
		void progress (const Domain& D, const BlasVector<OKDomain >& e)
		{
			// Radix shelves
			std::vector< double >::iterator  _dsz_it = RadixSizes_.begin();
			std::vector< LazyProduct >::iterator _mod_it = RadixPrimeProd_.begin();
			std::vector< BlasVector<Givaro::ZRing<Integer> > >::iterator _tab_it = RadixResidues_.begin();
			std::vector< bool >::iterator    _occ_it = RadixOccupancy_.begin();
			Givaro::ZRing<Integer> ZZ;
			LazyProduct mi; double di;
			BlasVector<Givaro::ZRing<Integer> > ri(ZZ,e.size());
			if (*_occ_it) {
				// If lower shelf is occupied
				// Combine it with the new residue
				// The for loop will try to put the resulting combination on the upper shelf
				typename BlasVector<Domain>::const_iterator  e_it = e.begin();
				BlasVector<Givaro::ZRing<Integer> >::iterator       ri_it = ri.begin();
				BlasVector<Givaro::ZRing<Integer> >::const_iterator t0_it = _tab_it->begin();
				DomainElement invP0; precomputeInvP0(invP0, D, _mod_it->operator()() );
				for( ; ri_it != ri.end(); ++e_it, ++ri_it, ++ t0_it)
					fieldreconstruct(*ri_it, D, *e_it, *t0_it, invP0, (*_mod_it).operator()() );
				Integer tmp; D.characteristic(tmp);
				double ltp = Givaro::naturallog(tmp);
				di = *_dsz_it + ltp;
				totalsize += ltp;
				mi.mulin(tmp);
				mi.mulin(*_mod_it);
				*_occ_it = false;
			}
			else {
				// Lower shelf is free
				// Put the new residue here and exit
				Integer tmp; D.characteristic(tmp);
				double ltp =  Givaro::naturallog(tmp);
				_mod_it->initialize(tmp);
				*_dsz_it = ltp;
				totalsize += ltp;
				typename BlasVector<Domain>::const_iterator e_it = e.begin();
				_tab_it->resize(e.size());
				BlasVector<Givaro::ZRing<Integer> >::iterator t0_it= _tab_it->begin();
				for( ; e_it != e.end(); ++e_it, ++ t0_it)
					D.convert(*t0_it, *e_it);
				*_occ_it = true;
				return;
			}

			// We have a combination to put in the upper shelf
			for(++_dsz_it, ++_mod_it, ++_tab_it, ++_occ_it ; _occ_it != RadixOccupancy_.end() ; ++_dsz_it, ++_mod_it, ++_tab_it, ++_occ_it) {
				if (*_occ_it) {
					// This shelf is occupied
					// Combine it with the new combination
					// The loop will try to put it on the upper shelf
					BlasVector<Givaro::ZRing<Integer> >::iterator      ri_it = ri.begin();
					BlasVector<Givaro::ZRing<Integer> >::const_iterator t_it= _tab_it->begin();

					Integer invprod; precomputeInvProd(invprod, mi(), _mod_it->operator()());
					for( ; ri_it != ri.end(); ++ri_it, ++ t_it)
						smallbigreconstruct(*ri_it, *t_it, invprod);

					// Product (lazy) computation
					mi.mulin(*_mod_it);

					// Moding out
					for(ri_it = ri.begin() ; ri_it != ri.end(); ++ri_it) {
						*ri_it %= mi();
					}

					di += *_dsz_it;
					*_occ_it = false;
				}
				else {
					// This shelf is free
					// Put the new combination here and exit
					*_dsz_it = di;
					*_mod_it = mi;
					*_tab_it = ri;
					*_occ_it = true;
					return;
				}
			}
			// All the shelfves were occupied
			// We create a new top shelf
			// And put the new combination there
			RadixSizes_.push_back( di );
			RadixResidues_.push_back( ri );
			RadixPrimeProd_.push_back( mi );
			RadixOccupancy_.push_back ( true );
		}


		//! result
		template<class Vect>
		Vect& result (Vect &d)
		{
			d.resize( (RadixResidues_.front()).size() );
			std::vector< LazyProduct >::iterator          _mod_it = RadixPrimeProd_.begin();
			std::vector< BlasVector< Givaro::ZRing<Integer> > >::iterator _tab_it = RadixResidues_.begin();
			std::vector< bool >::iterator                _occ_it = RadixOccupancy_.begin();
			LazyProduct Product;
			// We have to find to lowest occupied shelf
			for( ; _occ_it != RadixOccupancy_.end() ; ++_mod_it, ++_tab_it, ++_occ_it) {
				if (*_occ_it) {
					// Found the lowest occupied shelf
					Product = *_mod_it;
					BlasVector<Givaro::ZRing<Integer> >::iterator t0_it = d.begin();
					BlasVector<Givaro::ZRing<Integer> >::iterator t_it = _tab_it->begin();
					if (++_occ_it == RadixOccupancy_.end()) {
						// It is the only shelf of the radix
						// We normalize the result and output it
						for( ; t0_it != d.end(); ++t0_it, ++t_it)
							normalize(*t0_it = *t_it, *t_it, _mod_it->operator()());
						//RadixPrimeProd_.resize(1);
						return d;
					}
					else {
						// There are other shelves
						// The result is initialized with this shelf
						// The for loop will combine the other shelves m with the actual one
						for( ; t0_it != d.end(); ++t0_it, ++t_it)
							*t0_it  = *t_it;
						++_mod_it; ++_tab_it;
						break;
					}
				}
			}
			for( ; _occ_it != RadixOccupancy_.end() ; ++_mod_it, ++_tab_it, ++_occ_it) {
				if (*_occ_it) {
					// This shelf is occupied
					// We need to combine it with the actual value of the result
					BlasVector<Givaro::ZRing<Integer> >::iterator t0_it = d.begin();
					BlasVector<Givaro::ZRing<Integer> >::const_iterator t_it = _tab_it->begin();
					Integer invprod;
					precomputeInvProd(invprod, Product(), _mod_it->operator()() );

					for( ; t0_it != d.end(); ++t0_it, ++t_it)
						smallbigreconstruct(*t0_it, *t_it, invprod);

					// Overall product computation
					Product.mulin(*_mod_it);

					// Moding out and normalization
					for(t0_it = d.begin();t0_it != d.end(); ++t0_it) {
						*t0_it %= Product();
						Integer tmp(*t0_it);
						normalize(*t0_it, tmp, Product());
					}

				}
			}

			// We put it also the final prime product in the first shelf of products
			// JGD : should we also put the result
			//       in the first shelf of residues and resize it to 1
			//       and set to true the first occupancy and resize it to 1
			//       in case result is not the last call (more progress to go) ?
			RadixPrimeProd_.resize(1);
			RadixPrimeProd_.front() = Product;
			RadixSizes_.resize(1);
			RadixSizes_.front() =  Givaro::naturallog(Product());
			Givaro::ZRing<Integer> ZZ;
			RadixResidues_.resize(1,BlasVector<Givaro::ZRing<Integer> >(ZZ));
			RadixResidues_.front() = d;
			RadixOccupancy_.resize(1);
			RadixOccupancy_.front() = true;

			return d;
		}

		// spec for BlasVector
		BlasVector<Givaro::ZRing<Integer> >& result (BlasVector<Givaro::ZRing<Integer> > &d)
		{
			d.resize( (RadixResidues_.front()).size() );
			std::vector< LazyProduct >::iterator          _mod_it = RadixPrimeProd_.begin();
			std::vector< BlasVector< Givaro::ZRing<Integer> > >::iterator _tab_it = RadixResidues_.begin();
			std::vector< bool >::iterator                _occ_it = RadixOccupancy_.begin();
			LazyProduct Product;
			// We have to find to lowest occupied shelf
			for( ; _occ_it != RadixOccupancy_.end() ; ++_mod_it, ++_tab_it, ++_occ_it) {
				if (*_occ_it) {
					// Found the lowest occupied shelf
					Product = *_mod_it;
					BlasVector<Givaro::ZRing<Integer> >::iterator t0_it = d.begin();
					BlasVector<Givaro::ZRing<Integer> >::iterator t_it = _tab_it->begin();
					if (++_occ_it == RadixOccupancy_.end()) {
						// It is the only shelf of the radix
						// We normalize the result and output it
						for( ; t0_it != d.end(); ++t0_it, ++t_it)
							normalize(*t0_it = *t_it, *t_it, _mod_it->operator()());
						//RadixPrimeProd_.resize(1);
						return d;
					}
					else {
						// There are other shelves
						// The result is initialized with this shelf
						// The for loop will combine the other shelves m with the actual one
						for( ; t0_it != d.end(); ++t0_it, ++t_it)
							*t0_it  = *t_it;
						++_mod_it; ++_tab_it;
						break;
					}
				}
			}
			for( ; _occ_it != RadixOccupancy_.end() ; ++_mod_it, ++_tab_it, ++_occ_it) {
				if (*_occ_it) {
					// This shelf is occupied
					// We need to combine it with the actual value of the result
					BlasVector<Givaro::ZRing<Integer> >::iterator t0_it = d.begin();
					BlasVector<Givaro::ZRing<Integer> >::const_iterator t_it = _tab_it->begin();
					Integer invprod;
					precomputeInvProd(invprod, Product(), _mod_it->operator()() );

					for( ; t0_it != d.end(); ++t0_it, ++t_it)
						smallbigreconstruct(*t0_it, *t_it, invprod);

					// Overall product computation
					Product.mulin(*_mod_it);

					// Moding out and normalization
					for(t0_it = d.begin();t0_it != d.end(); ++t0_it) {
						*t0_it %= Product();
						Integer tmp(*t0_it);
						normalize(*t0_it, tmp, Product());
					}

				}
			}

			// We put it also the final prime product in the first shelf of products
			// JGD : should we also put the result
			//       in the first shelf of residues and resize it to 1
			//       and set to true the first occupancy and resize it to 1
			//       in case result is not the last call (more progress to go) ?
			RadixPrimeProd_.resize(1);
			RadixPrimeProd_.front() = Product;
			RadixSizes_.resize(1);
			RadixSizes_.front() =  Givaro::naturallog(Product());
			Givaro::ZRing<Integer> Z;
			RadixResidues_.resize(1,BlasVector<Givaro::ZRing<Integer> >(Z));
			RadixResidues_.front() = d;
			RadixOccupancy_.resize(1);
			RadixOccupancy_.front() = true;

			return d;
		}


		bool terminated()
		{
			return totalsize > LOGARITHMIC_UPPER_BOUND;
		}

		bool noncoprime(const Integer& i) const
		{
			std::vector< LazyProduct >::const_iterator _mod_it = RadixPrimeProd_.begin();
			std::vector< bool >::const_iterator    _occ_it = RadixOccupancy_.begin();
			for( ; _occ_it != RadixOccupancy_.end() ; ++_mod_it, ++_occ_it)
				if ((*_occ_it) && (_mod_it->noncoprime(i))) return true;
			return false;
		}



	protected:

		Integer& precomputeInvProd(Integer& res, const Integer& m1, const Integer& m0)
		{
			inv(res, m0, m1);
			return res *= m0; // res <-- (m0^{-1} mod m1)*m0
		}

		DomainElement& precomputeInvP0(DomainElement& invP0, const Domain& D1, const Integer& P0)
		{
			return D1.invin( D1.init(invP0, P0) ); // res <-- (P0^{-1} mod m1)
		}


		Integer& smallbigreconstruct(Integer& u1, const Integer& u0, const Integer& invprod)
		{
			u1 -= u0;	  // u1 <-- (u1-u0)
			u1 *= invprod;    // u1 <-- (u1-u0)( m0^{-1} mod m1 ) m0
			return u1 += u0;  // u1 <-- u0 + (u1-u0)( m0^{-1} mod m1 ) m0
		}


		Integer& normalize(Integer& u1, Integer& tmp, const Integer& m1)
		{
			if (u1 < 0)
				tmp += m1;
			else
				tmp -= m1;
			return ((absCompare(u1,tmp) > 0)? u1 = tmp : u1 );
		}


		Integer& fieldreconstruct(Integer& res, const Domain& D1, const DomainElement& u1, const Integer& r0, const DomainElement& invP0, const Integer& P0)
		{
                    	// u0 = r0 mod m1
			DomainElement u0; D1.init(u0, r0);
			if (D1.areEqual(u1, u0))
				return res=r0;
			else
				return fieldreconstruct(res, D1, u1, u0, r0, invP0, P0);
		}

		Integer& fieldreconstruct(Integer& res, const Domain& D1, const DomainElement& u1, DomainElement& u0, const Integer& r0, const DomainElement& invP0, const Integer& P0)
		{
			// u0 and m0 are modified
			D1.negin(u0);   	// u0 <-- -u0
			D1.addin(u0,u1);   	// u0 <-- u1-u0
			D1.mulin(u0, invP0);    // u0 <-- (u1-u0)( m0^{-1} mod m1 )
			D1.convert(res, u0);    // res <-- (u1-u0)( m0^{-1} mod m1 )         and res <  m1
			res *= P0;      	// res <-- (u1-u0)( m0^{-1} mod m1 ) m0      and res <= (m0m1-m0)
			return res += r0;	// res <-- u0 + (u1-u0)( m0^{-1} mod m1 ) m0 and res <  m0m1
		}

	};

}


#endif //__LINBOX_cra_full_multip_H

// Local Variables:
// mode: C++
// tab-width: 4
// indent-tabs-mode: nil
// c-basic-offset: 4
// End:
// vim:sts=4:sw=4:ts=4:et:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
