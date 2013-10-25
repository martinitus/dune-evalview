//**************************************************************************************//
//     AUTHOR: Malik Kirchner "malik.kirchner@gmx.net"                                  //
//             Martin Rückl "martin.rueckl@physik.hu-berlin.de"                         //
//                                                                                      //
//     This program is free software: you can redistribute it and/or modify             //
//     it under the terms of the GNU General Public License as published by             //
//     the Free Software Foundation, either version 3 of the License, or                //
//     (at your option) any later version.                                              //
//                                                                                      //
//     This program is distributed in the hope that it will be useful,                  //
//     but WITHOUT ANY WARRANTY; without even the implied warranty of                   //
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    //
//     GNU General Public License for more details.                                     //
//                                                                                      //
//     You should have received a copy of the GNU General Public License                //
//     along with this program.  If not, see <http://www.gnu.org/licenses/>.            //
//                                                                                      //
//     Dieses Programm ist Freie Software: Sie können es unter den Bedingungen          //
//     der GNU General Public License, wie von der Free Software Foundation,            //
//     Version 3 der Lizenz oder (nach Ihrer Option) jeder späteren                     //
//     veröffentlichten Version, weiterverbreiten und/oder modifizieren.                //
//                                                                                      //
//     Dieses Programm wird in der Hoffnung, dass es nützlich sein wird, aber           //
//     OHNE JEDE GEWÄHRLEISTUNG, bereitgestellt; sogar ohne die implizite               //
//     Gewährleistung der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.       //
//     Siehe die GNU General Public License für weitere Details.                        //
//                                                                                      //
//     Sie sollten eine Kopie der GNU General Public License zusammen mit diesem        //
//     Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.       //
//                                                                                      //
//**************************************************************************************//

#pragma once

#include <limits>
#include <vector>
#include <map>

#include <fem/helper.hpp>
#include <tree/node.hpp>
#include <tree/leafview.hpp>
#include <tree/levelview.hpp>



namespace tree {


template< class GV >
class Root : public Node<GV> {
public:
    typedef typename Node<GV>::Traits Traits;
    
protected:
    using Node<GV>::_parent;
    using Node<GV>::_gridView;
    using Node<GV>::_grid;
    using Node<GV>::_vertex;
    using Node<GV>::_bounding_box;
    using Node<GV>::split;
    using Node<GV>::put;
    using Node<GV>::findNode;

    
    typedef typename Node<GV>::Vertex           Vertex;
    typedef typename Traits::Real               Real;
    typedef typename Traits::Entity             Entity;
    typedef typename Traits::EntitySeed         EntitySeed;
    typedef typename Traits::EntityPointer      EntityPointer;
    typedef typename Traits::GridView           GridView;
    typedef typename Traits::LinaVector         LinaVector;
    
    static constexpr unsigned dim     = Traits::dim;
    static constexpr unsigned dimw    = Traits::dimw;
    
protected:
    std::vector<EntitySeed>     _entities;

public:
    Root( const Root<GridView>& root ) {};
    
    virtual ~Root( ) {
        for ( auto v : _vertex )
            safe_delete( v );
        _vertex.clear();
    };

    Root( const GridView& gridview ) :
        Node<GV>(NULL,gridview)
    {
        std::vector< Vertex* > _l_vertex;
        // create container of all entity seeds
        for( auto e = gridview.template begin<0>(); e != gridview.template end<0>(); ++e ) {
            _entities.push_back( e->seed() );
            auto geo = e->geometry();
            
            auto& gidSet    = _gridView.grid().globalIdSet();
            auto& gidxSet   = _gridView.grid().leafIndexSet();
            
            for ( unsigned k = 0; k < geo.corners(); k++ ) {
                typename Traits::LinaVector gl = fem::asShortVector<Real, dim>( geo.corner(k) ) ;
                
                Vertex* _v = NULL;
                
                for ( auto vl = _l_vertex.begin(); vl != _l_vertex.end(); ++vl ) {
                    if ( math::norm2((*vl)->_global - gl) < 10.*std::numeric_limits<Real>::epsilon() )
                        _v = *vl;
                }                
                
                if ( _v == NULL ) { 
                    _v = new Vertex();
                    _l_vertex.push_back( _v ); // TODO: use mapping to avoid duplication!!!!
                    _bounding_box.append(gl);
                }                

                // store global coordinates of all vertices
                _v->_global = gl;
                _v->_entity_seed.push_back( &(_entities.back()) );
            }
        }

        std::cout << "Bounding box\n" ; _bounding_box.operator<<(std::cout) << std::endl;
        std::cout << "Number of vertices " << _l_vertex.size() << std::endl;
        
        // generate list of vertices
        this->put( _l_vertex.begin(), _l_vertex.end() );
    }

     // iterate over all leafs of the node
    LeafView<GridView> leafView() const {
        return LeafView<GridView>( *this );
    }

     // iterate over all leafs of the node
    LevelView<GridView> levelView(unsigned level) const {
        return LevelView<GridView>( *this, level );
    }

    
    virtual void fillTreeStats( typename Node<GridView>::TreeStats& ts ) const {
        Node<GV>::fillTreeStats(ts);
        
        ts.numVertices         = _vertex.size();
        ts.aveLevel           /= static_cast<Real>( ts.numNodes );
        ts.aveLeafLevel       /= static_cast<Real>( ts.numLeafs );
        ts.aveVertices        /= static_cast<Real>( ts.numNodes );
        ts.aveEntitiesPerLeaf /= static_cast<Real>( ts.numLeafs );
    }
    
    void printTreeStats( std::ostream& out ) const {
        typename Node<GridView>::TreeStats ts;
        fillTreeStats(ts);
        ts.operator<<(out) << std::endl;
    }
    
    
    const EntityPointer findEntity( const LinaVector& x ) const {
        // find node containing all possible cells
        const Node<GridView>* node = findNode( x );
        
#ifndef NDEBUG
//         if ( !node         ) return EntityPointer();
//         if ( !node->_empty ) return EntityPointer();
#endif

        // iterate cells and return containing cell
        auto xg = fem::asFieldVector( x );
        for ( auto es : node->vertex(0)->_entity_seed ) {
            const EntityPointer ep( _grid.entityPointer( *es ) );
            const Entity&   e   = *ep; 
            const auto&     geo = e.geometry();   
            const auto&     gre = Dune::GenericReferenceElements< Real, dim >::general(geo.type());
            const auto      xl  = geo.local( xg );
            if ( gre.checkInside( xl ) )
                return ep;            
        }
        
        throw ;
    }    
};


}
