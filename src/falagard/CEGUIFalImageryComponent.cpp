/************************************************************************
    filename:   CEGUIFalImageryComponent.cpp
    created:    Mon Jun 13 2005
    author:     Paul D Turner <paul@cegui.org.uk>
*************************************************************************/
/*************************************************************************
    Crazy Eddie's GUI System (http://www.cegui.org.uk)
    Copyright (C)2004 - 2005 Paul D Turner (paul@cegui.org.uk)
 
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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*************************************************************************/
#include "falagard/CEGUIFalImageryComponent.h"
#include "CEGUIImage.h"
#include "CEGUIExceptions.h"
#include "CEGUIImagesetManager.h"
#include "CEGUIImageset.h"
#include "CEGUIPropertyHelper.h"

// void	draw(const Rect& dest_rect, float z, const Rect& clip_rect,const ColourRect& colours);

// Start of CEGUI namespace section
namespace CEGUI
{
    ImageryComponent::ImageryComponent() :
        d_image(0),
		d_colours(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF),
        d_vertFormatting(VF_TOP_ALIGNED),
        d_horzFormatting(HF_LEFT_ALIGNED),
        d_colourProperyIsRect(false)
    {}

    ImageryComponent::ImageryComponent(const ComponentArea& area, const String& imageset, const String& image, const ColourRect& cols, VerticalFormatting vFmt, HorizontalFormatting hFmt) :
        d_area(area),
        d_colours(cols),
        d_vertFormatting(vFmt),
        d_horzFormatting(hFmt),
        d_colourProperyIsRect(false)
    {
        setImage(imageset, image);
    }

    ImageryComponent::ImageryComponent(const ComponentArea& area, const Image* image, const ColourRect& cols, VerticalFormatting vFmt, HorizontalFormatting hFmt) :
        d_area(area),
        d_image(image),
        d_colours(cols),
        d_vertFormatting(vFmt),
        d_horzFormatting(hFmt),
        d_colourProperyIsRect(false)
    {}

    void ImageryComponent::render(Window& srcWindow, float base_z, const CEGUI::ColourRect* modColours, const Rect* clipper) const
    {
        Rect destRect(d_area.getPixelRect(srcWindow));
        render_impl(srcWindow, destRect, base_z, modColours, clipper);
    }

    void ImageryComponent::render(Window& srcWindow, const Rect& baseRect, float base_z, const CEGUI::ColourRect* modColours, const Rect* clipper) const
    {
        Rect destRect(d_area.getPixelRect(srcWindow, baseRect));
        render_impl(srcWindow, destRect, base_z, modColours, clipper);
    }

    const ComponentArea& ImageryComponent::getComponentArea() const
    {
        return d_area;
    }

    void ImageryComponent::setComponentArea(const ComponentArea& area)
    {
        d_area = area;
    }

    const Image* ImageryComponent::getImage() const
    {
        return d_image;
    }

    void ImageryComponent::setImage(const Image* image)
    {
        d_image = image;
    }

    void ImageryComponent::setImage(const String& imageset, const String& image)
    {
        try
        {
            d_image = &ImagesetManager::getSingleton().getImageset(imageset)->getImage(image);
        }
        catch (UnknownObjectException)
        {
            d_image = 0;
        }
    }

    const ColourRect& ImageryComponent::getColours() const
    {
        return d_colours;
    }

    void ImageryComponent::setColours(const ColourRect& cols)
    {
        d_colours = cols;
    }

    VerticalFormatting ImageryComponent::getVerticalFormatting() const
    {
        return d_vertFormatting;
    }

    void ImageryComponent::setVerticalFormatting(VerticalFormatting fmt)
    {
        d_vertFormatting = fmt;
    }

    HorizontalFormatting ImageryComponent::getHorizontalFormatting() const
    {
        return d_horzFormatting;
    }

    void ImageryComponent::setHorizontalFormatting(HorizontalFormatting fmt)
    {
        d_horzFormatting = fmt;
    }

    void ImageryComponent::setColoursPropertySource(const String& property)
    {
        d_colourPropertyName = property;
    }

    void ImageryComponent::setColoursPropertyIsColourRect(bool setting)
    {
        d_colourProperyIsRect = setting;
    }

    void ImageryComponent::initColoursRect(const Window& wnd, ColourRect& cr) const
    {
        // if colours come via a colour property
        if (!d_colourPropertyName.empty())
        {
            // if property accesses a ColourRect
            if (d_colourProperyIsRect)
            {
                cr = PropertyHelper::stringToColourRect(wnd.getProperty(d_colourPropertyName));
            }
            // property accesses a colour
            else
            {
                colour val(PropertyHelper::stringToColour(wnd.getProperty(d_colourPropertyName)));
                cr.d_top_left     = val;
                cr.d_top_right    = val;
                cr.d_bottom_left  = val;
                cr.d_bottom_right = val;
            }
        }
        // use explicit ColourRect.
        else
        {
            cr = d_colours;
        }
    }

    void ImageryComponent::render_impl(Window& srcWindow, const Rect& destRect, float base_z, const CEGUI::ColourRect* modColours, const Rect* clipper) const
    {
        // do not draw anything if image is not set.
        if (!d_image)
            return;

        uint horzTiles, vertTiles;
        float xpos, ypos;

        Size imgSz(d_image->getSize());

        // calculate final colours to be used
        ColourRect finalColours;
        initColoursRect(srcWindow, finalColours);
        if (modColours)
        {
            finalColours *= *modColours;
        }

        // calculate initial x co-ordinate and horizontal tile count according to formatting options
        switch (d_horzFormatting)
        {
            case HF_STRETCHED:
                imgSz.d_width = destRect.getWidth();
                xpos = destRect.d_left;
                horzTiles = 1;
                break;

            case HF_TILED:
                xpos = destRect.d_left;
                horzTiles = (uint)((destRect.getWidth() + (imgSz.d_width - 1)) / imgSz.d_width);
                break;

            case HF_LEFT_ALIGNED:
                xpos = destRect.d_left;
                horzTiles = 1;
                break;

            case HF_CENTRE_ALIGNED:
                xpos = destRect.d_left + PixelAligned((destRect.getWidth() - imgSz.d_width) * 0.5f);
                horzTiles = 1;
                break;

            case HF_RIGHT_ALIGNED:
                xpos = destRect.d_right - imgSz.d_width;
                horzTiles = 1;
                break;

            default:
                throw InvalidRequestException("ImageryComponent::render - An unknown HorizontalFormatting value was specified.");
        }

        // calculate initial y co-ordinate and vertical tile count according to formatting options
        switch (d_vertFormatting)
        {
            case VF_STRETCHED:
                imgSz.d_height = destRect.getHeight();
                ypos = destRect.d_top;
                vertTiles = 1;
                break;

            case VF_TILED:
                ypos = destRect.d_top;
                vertTiles = (uint)((destRect.getHeight() + (imgSz.d_height - 1)) / imgSz.d_height);
                break;

            case VF_TOP_ALIGNED:
                ypos = destRect.d_top;
                vertTiles = 1;
                break;

            case VF_CENTRE_ALIGNED:
                ypos = destRect.d_top + PixelAligned((destRect.getHeight() - imgSz.d_height) * 0.5f);
                vertTiles = 1;
                break;

            case VF_BOTTOM_ALIGNED:
                ypos = destRect.d_bottom - imgSz.d_height;
                vertTiles = 1;
                break;

            default:
                throw InvalidRequestException("ImageryComponent::render - An unknown VerticalFormatting value was specified.");
        }

        // perform final rendering (actually is now a caching of the images which will be drawn)
        Rect finalRect;
        finalRect.d_top = ypos;
        finalRect.d_bottom = ypos + imgSz.d_height;

        for (uint row = 0; row < vertTiles; ++row)
        {
            finalRect.d_left = xpos;
            finalRect.d_right = xpos + imgSz.d_width;

            for (uint col = 0; col < horzTiles; ++col)
            {
                // add image to the rendering cache for the target window.
                srcWindow.getRenderCache().cacheImage(*d_image, finalRect, base_z, finalColours, clipper);

                finalRect.d_left += imgSz.d_width;
                finalRect.d_right += imgSz.d_width;
            }

            finalRect.d_top += imgSz.d_height;
            finalRect.d_bottom += imgSz.d_height;
        }
    }

} // End of  CEGUI namespace section
