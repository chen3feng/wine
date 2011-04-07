/*
 * DIB driver GDI objects.
 *
 * Copyright 2011 Huw Davies
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "gdi_private.h"
#include "dibdrv.h"

#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(dib);

/***********************************************************************
 *           dibdrv_SelectPen
 */
HPEN CDECL dibdrv_SelectPen( PHYSDEV dev, HPEN hpen )
{
    PHYSDEV next = GET_NEXT_PHYSDEV( dev, pSelectPen );
    dibdrv_physdev *pdev = get_dibdrv_pdev(dev);
    LOGPEN logpen;

    TRACE("(%p, %p)\n", dev, hpen);

    if (!GetObjectW( hpen, sizeof(logpen), &logpen ))
    {
        /* must be an extended pen */
        EXTLOGPEN *elp;
        INT size = GetObjectW( hpen, 0, NULL );

        if (!size) return 0;

        elp = HeapAlloc( GetProcessHeap(), 0, size );

        GetObjectW( hpen, size, elp );
        /* FIXME: add support for user style pens */
        logpen.lopnStyle = elp->elpPenStyle;
        logpen.lopnWidth.x = elp->elpWidth;
        logpen.lopnWidth.y = 0;
        logpen.lopnColor = elp->elpColor;

        HeapFree( GetProcessHeap(), 0, elp );
    }

    if (hpen == GetStockObject( DC_PEN ))
        logpen.lopnColor = GetDCPenColor( dev->hdc );

    pdev->pen_color = pdev->dib.funcs->colorref_to_pixel(&pdev->dib, logpen.lopnColor);

    pdev->defer |= DEFER_PEN;

    switch(logpen.lopnStyle & PS_STYLE_MASK)
    {
    case PS_SOLID:
        if(logpen.lopnStyle & PS_GEOMETRIC) break;
        if(logpen.lopnWidth.x > 1) break;
        pdev->defer &= ~DEFER_PEN;
        break;
    default:
        break;
    }

    return next->funcs->pSelectPen( next, hpen );
}

/***********************************************************************
 *           dibdrv_SetDCPenColor
 */
COLORREF CDECL dibdrv_SetDCPenColor( PHYSDEV dev, COLORREF color )
{
    PHYSDEV next = GET_NEXT_PHYSDEV( dev, pSetDCPenColor );
    dibdrv_physdev *pdev = get_dibdrv_pdev(dev);

    if (GetCurrentObject(dev->hdc, OBJ_PEN) == GetStockObject( DC_PEN ))
        pdev->pen_color = pdev->dib.funcs->colorref_to_pixel(&pdev->dib, color);

    return next->funcs->pSetDCPenColor( next, color );
}
