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

#include <node.hpp>



namespace evalview {


template< class GV >
class Root : public Node<GV> {
protected:
    using Node<GV>::_father;
    using Node<GV>::_gridview;
    using Node<GV>::_bounding_box;

    typedef typename Node<GV>::Traits Traits;

protected:

    std::vector<EntitySeed> _entities;

    struct
    {
        std::vector<unsigned> _element_ids;
        unsigned _vertex_id;
    };

    // map each vertex id to its corresponding entity index. where entity index is the
    // index of the entity seed for the container above.
    std::map<unsigned, std::vector<unsigned>> _maping;

public:
    Root( const Root& node ) = default;

    Root( const typename Traits::GridView& gridview ) :
        Node<GV>(NULL,gridview)/*, _father(NULL), _gridview(gridview)*/
    {
        // create container of all entity seeds
        for(Entity e : gridview.elements())
        {
            std::size_t pos = _entities.push_back(EntitySeed(e));

            for (Vertices v : e.vertices())
            {
                _maping(v.index()).push_back(pos);

                _bounding_box.append(v.global());
            }
        }


    }

     // iterate over all leafs of the node
    LeafView leafView() const
    {
        return LeafView(*this);
    }

     // iterate over all leafs of the node
    LevelView levelView(unsigned level) const
    {
        return LevelView(*this,level);
    }

};


}
