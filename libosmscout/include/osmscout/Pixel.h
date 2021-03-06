#ifndef OSMSCOUT_PIXEL_H
#define OSMSCOUT_PIXEL_H

/*
  This source is part of the libosmscout library
  Copyright (C) 2013  Tim Teulings

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/

#include <osmscout/private/CoreImportExport.h>

#include <osmscout/system/Types.h>

namespace osmscout {

  struct OSMSCOUT_API Pixel
  {
    uint32_t x;
    uint32_t y;

    inline Pixel()
    {
      // no code
    }

    inline Pixel(uint32_t x, uint32_t y)
     :x(x),y(y)
    {
      // no code
    }

    inline bool operator==(const Pixel& other) const
    {
      return x==other.x && y==other.y;
    }

    inline bool operator<(const Pixel& other) const
    {
      return y<other.y ||
      (y==other.y && x<other.x);
    }
  };

  struct OSMSCOUT_API Vertex2D
  {
  private:
    double coords[2];

  public:
    inline Vertex2D()
    {
      // no code
    }

    inline Vertex2D(const Vertex2D& other)
    {
      coords[0]=other.coords[0];
      coords[1]=other.coords[1];
    }

    inline Vertex2D(double x,
                    double y)
    {
      coords[0]=x;
      coords[1]=y;
    }

    inline void SetX(double x)
    {
      coords[0]=x;
    }

    inline void SetY(double y)
    {
      coords[1]=y;
    }

    inline void Set(double x,
                    double y)
    {
      coords[0]=x;
      coords[1]=y;
    }

    inline double GetX() const
    {
      return coords[0];
    }

    inline double GetY() const
    {
      return coords[1];
    }

    inline bool operator==(const Vertex2D& other) const
    {
      return coords[0]==other.coords[0] &&
             coords[1]==other.coords[1];
    }

    inline bool operator<(const Vertex2D& other) const
    {
      return coords[1]<other.coords[1] ||
             (coords[1]==other.coords[1] && coords[0]<other.coords[0]);
    }
  };

  struct OSMSCOUT_API Vertex3D
  {
  private:
    double x;
    double y;
    double z;

  public:
    inline Vertex3D()
    {
      // no code
    }

    inline Vertex3D(const Vertex3D& other)
     :x(other.x),
      y(other.y),
      z(other.z)
    {
      // no code
    }

    inline Vertex3D(double x,
                    double y)
     :x(x),
      y(y),
      z(0.0)

    {
      // no code
    }

    inline double GetX() const
    {
      return x;
    }

    inline double GetY() const
    {
      return y;
    }

    inline double GetZ() const
    {
      return y;
    }

    inline void SetX(double x)
    {
      this->x=x;
    }

    inline void SetY(double y)
    {
      this->y=y;
    }

    inline void SetZ(double z)
    {
      this->z=z;
    }

    inline void Set(double x,
                    double y)
    {
      this->x=x;
      this->y=y;
      this->z=0;
    }

    inline void Set(double x,
                    double y,
                    double z)
    {
      this->x=x;
      this->y=y;
      this->z=z;
    }

    inline bool operator==(const Vertex3D& other) const
    {
      return x==other.x &&
             y==other.y &&
             z==other.z;
    }

    inline bool operator<(const Vertex3D& other) const
    {
      if (x!=other.x) {
        return x<other.x;
      }
      else if (y!=other.y) {
        return y<other.y;
      }

      return z<other.z;
    }
  };
}

#endif
