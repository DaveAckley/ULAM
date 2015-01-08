/**                                        -*- mode:C++ -*-
 * NodeTypeBitsize.h - Basic Node handling Type Bitsizes for ULAM
 *
 * Copyright (C) 2014 The Regents of the University of New Mexico.
 * Copyright (C) 2014 Ackleyshack LLC.
 *
 * This file is part of the ULAM programming language compilation system.
 *
 * The ULAM programming language compilation system is free software:
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * The ULAM programming language compilation system is distributed in
 * the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the ULAM programming language compilation system
 * software.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 */

/**
  \file NodeTypeBitsize.h - Basic Node handling Type Bitsizes for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/


#ifndef NODETYPEBITSIZE_H
#define NODETYPEBITSIZE_H

#include "Node.h"

namespace MFM{

  class NodeTypeBitsize : public Node
  {
  public:

    NodeTypeBitsize(Node * node, CompilerState & state);
    ~NodeTypeBitsize();

    virtual void updateLineage(Node * p);

    virtual void printPostfix(File * f);

    virtual UTI checkAndLabelType();

    virtual EvalStatus eval();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    bool getTypeBitSizeInParen(s32& rtnBitSize, ULAMTYPE BUT);

  private:
    Node * m_node;
  };

}

#endif //end NODETYPEBITSIZE_H
