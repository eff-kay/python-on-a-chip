/*
 * PyMite - A flyweight Python interpreter for 8-bit microcontrollers and more.
 * Copyright 2002 Dean Hall
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#undef __FILE_ID__
#define __FILE_ID__ 0x05
/**
 * VM Globals
 *
 * VM globals operations.
 * Py's global struct def and initial values.
 *
 * Log
 * ---
 *
 * 2006/09/10   #20: Implement assert statement
 * 2006/08/29   #12: Make mem_*() funcs use RAM when target is DESKTOP
 * 2006/08/29   #15 - All mem_*() funcs and pointers in the vm should use
 *              unsigned not signed or void
 * 2002/04/22   First.
 */

/***************************************************************
 * Includes
 **************************************************************/

#include "py.h"


/***************************************************************
 * Constants
 **************************************************************/

/***************************************************************
 * Macros
 **************************************************************/

/***************************************************************
 * Types
 **************************************************************/

/***************************************************************
 * Globals
 **************************************************************/

/** Most PyMite globals all in one convenient place */
PyVmGlobal_t gVmGlobal;


/***************************************************************
 * Functions
 **************************************************************/

PyReturn_t
global_init(void)
{
    PyReturn_t retval;
    P_U8 codestr = (P_U8)"code";

    /* clear the global struct less the heap */
    sli_memset((P_U8)&gVmGlobal, '\0', sizeof(PyVmGlobal_t) - sizeof(PyHeap_t));

    /* set the PyMite release num (for debug and post mortem) */
    gVmGlobal.errVmRelease = PY_RELEASE;

    /* init zero */
    gVmGlobal.zero.od.od_type = OBJ_TYPE_INT;
    gVmGlobal.zero.od.od_size = sizeof(PyInt_t);
    gVmGlobal.zero.od.od_const = 1;
    gVmGlobal.zero.val = 0;

    /* init one */
    gVmGlobal.one.od.od_type = OBJ_TYPE_INT;
    gVmGlobal.one.od.od_size = sizeof(PyInt_t);
    gVmGlobal.one.od.od_const = 1;
    gVmGlobal.one.val = 1;

    /* init negone */
    gVmGlobal.negone.od.od_type = OBJ_TYPE_INT;
    gVmGlobal.negone.od.od_size = sizeof(PyInt_t);
    gVmGlobal.negone.od.od_const = 1;
    gVmGlobal.negone.val = (S32)-1;

    /* init None */
    gVmGlobal.none.od.od_type = OBJ_TYPE_NON;
    gVmGlobal.none.od.od_size = sizeof(PyObj_t);
    gVmGlobal.none.od.od_const = 1;

    /* Init "code" string obj */
    retval = string_new((P_U8 *)&codestr, (pPyObj_t *)&gVmGlobal.pcodeStr);

    /* init empty builtins */
    gVmGlobal.builtins = C_NULL;

    /* empty img info list */
    gVmGlobal.pimglist = C_NULL;

    /* clear ptrs */
    /*FP = C_NULL;*//* fp is local to interp, until thread struct is made */

    /* interpreter loop return value */
    gVmGlobal.interpctrl = INTERP_CTRL_CONT;

    return retval;
}


PyReturn_t
global_loadBuiltins(pPyFunc_t pmod)
{
    PyReturn_t retval = PY_RET_OK;
    pPyObj_t pkey = C_NULL;
    P_U8 bistr = (P_U8)"__bi";
    P_U8 nonestr = (P_U8)"None";
    pPyObj_t pstr = C_NULL;
    pPyObj_t pbimod;

    /* import the builtins */
    retval = string_new(&bistr, &pstr);
    PY_RETURN_IF_ERROR(retval);
    retval = mod_import(pstr, &pbimod);
    PY_RETURN_IF_ERROR(retval);

    /* must interpret builtins' root code to set the attrs */
    retval = interpret((pPyFunc_t)pbimod);
    PY_RETURN_IF_ERROR(retval);

    /* reset interpreter to run */
    gVmGlobal.interpctrl = INTERP_CTRL_CONT;

    /* builtins points to the builtins module's attrs dict */
    gVmGlobal.builtins = ((pPyFunc_t)pbimod)->f_attrs;

    /* set None manually */
    retval = string_new(&nonestr, &pkey);
    PY_RETURN_IF_ERROR(retval);
    retval = dict_setItem(PY_PBUILTINS, pkey, PY_NONE);
    PY_RETURN_IF_ERROR(retval);

    /* put builtins module in the module's attrs dict */
    retval = string_new(&bistr, &pkey);
    PY_RETURN_IF_ERROR(retval);
    retval = dict_setItem((pPyObj_t)pmod->f_attrs, pkey, PY_PBUILTINS);
    PY_RETURN_IF_ERROR(retval);

    /* deallocate builtins module */
    heap_freeChunk((pPyObj_t)pbimod);

    return PY_RET_OK;
}

/***************************************************************
 * Main
 **************************************************************/

/***************************************************************
 * Test
 **************************************************************/
