#!/usr/bin/env python3
"""ATS input converter from main development branch to new-state development branch"""

import sys, os
try:
    amanzi_xml = os.path.join(os.environ["AMANZI_SRC_DIR"], "tools","amanzi_xml")
except KeyError:
    pass
else:
    if amanzi_xml not in sys.path:
        sys.path.append(amanzi_xml)

ats_tools = os.path.join(os.environ["ATS_SRC_DIR"], "tools", "utils")
if ats_tools not in sys.path:
    sys.path.append(ats_tools)

from amanzi_xml.utils import search as asearch
from amanzi_xml.utils import io as aio
from amanzi_xml.utils import errors as aerrors
from amanzi_xml.common import parameter, parameter_list
from convert_parameters_vg2bc import get_bc_param_from_vg

def add_soil_resistance(xml, rs_option="sakagucki-zeng soil resistance"):
    # rs_option can be "sakagucki-zeng soil resistance" or "sellers soil resistance"
    eval_list = asearch.find_path(xml, ["state", "evaluators"])
    try:
        rs_list = asearch.child_by_name(eval_list, "surface-soil_resistance")
    except aerrors.MissingXMLError:
        rs_list = eval_list.sublist("surface-soil_resistance")
        rs_list.append(parameter.StringParameter("evaluator type", rs_option))
        rs_list.append(parameter.StringParameter("model parameters", "WRM parameters"))
    else:
        return

def add_rel_perm(xml, relp_option="WRM rel perm"):
    eval_list = asearch.find_path(xml, ["state", "evaluators"])
    try:
        relp_list = asearch.child_by_name(eval_list, "relative_permeability")
    except aerrors.MissingXMLError:
        relp_list = eval_list.sublist("relative_permeability")
        relp_list.append(parameter.StringParameter("evaluator type", relp_option))
        if relp_option == "WRM rel perm":
            relp_list.append(parameter.StringParameter("model parameters", "WRM parameters"))
        else:
            assert(relp_option == "Brooks-Corey based high frozen rel perm")
            relp_list.append(parameter.StringParameter("model parameters", "freezing rel perm parameters"))
            relp_list.append(parameter.DoubleParameter("omega [-]", 2))
    else:
        return


def add_state_model_parameters(xml):
    state_list = asearch.find_path(xml, ["state",])
    try:
        model_par_list = asearch.child_by_name(state_list, "model parameters")
    except aerrors.MissingXMLError:
        model_par_list = state_list.sublist("model parameters")
    else:
        return


def del_lc_params(xml):
    lc_list = asearch.find_path(xml, ["state", "initial conditions", "land cover types"], no_skip=True)
    sub_lists = asearch.children_by_tag(lc_list, 'ParameterList')
    for s in sub_lists:
        try:
            s.remove(asearch.child_by_name(s, "dessicated zone thickness [m]"))
            s.remove(asearch.child_by_name(s, "Clapp and Hornberger b [-]"))
        except aerrors.MissingXMLError:
            pass


def add_wrm_to_model_parameters(xml):
    wrm_list = asearch.find_path(xml, ["PKs", "subsurface flow", "water retention evaluator"])
    try:
        mpar = asearch.child_by_name(wrm_list, "model parameters")
    except aerrors.MissingXMLError:
        wrm_list.append(parameter.StringParameter("model parameters", "WRM parameters"))
    else:
        mpar.setValue("WRM parameters")

    model_par_list = asearch.find_path(xml, ["state", "model parameters"], no_skip=True)
    if not model_par_list.isElement("WRM parameters"):
        wrm_param_list = asearch.remove_element(xml, 
            ["PKs", "subsurface flow", "water retention evaluator", "WRM parameters"], False, True)
        model_par_list.append(wrm_param_list)
        
    else:
        return


def add_dessicated_zone_to_WRM(xml, dessicated_zone_set):
    # dict: dessicated_zone_set = {key: soil_type, val: dessicated_zone_thickness}
    # list: dessicated_zone_set = [] equal length with soil types
    # single value: all soil types use the same dessicated_zone_thickness
    wrm_param_list = asearch.find_path(xml, ["state", "model parameters", "WRM parameters"], no_skip=True)
    sub_list = asearch.children_by_tag(wrm_param_list, 'ParameterList')
    if type(dessicated_zone_set) is list:
        assert(len(dessicated_zone_set) == len(sub_list))
        for s in sub_list:
            if not s.isElement("dessicated zone thickness [m]"):
                s.append(parameter.DoubleParameter("dessicated zone thickness [m]", 
                         dessicated_zone_set[sub_list.index(s)]))
            elif asearch.child_by_name(s, "dessicated zone thickness [m]").getValue() \
                != dessicated_zone_set[sub_list.index(s)]:
                s.remove(asearch.child_by_name(s, "dessicated zone thickness [m]"))
                s.append(parameter.DoubleParameter("dessicated zone thickness [m]", 
                         dessicated_zone_set[sub_list.index(s)]))
            else:
                pass
    else:
        for s in sub_list:
            if not s.isElement("dessicated zone thickness [m]"):
                s.append(parameter.DoubleParameter("dessicated zone thickness [m]", dessicated_zone_set)) 
        

def add_frz_relp_to_model_parameters(xml):
    def copy_element(a_soil_list, var_name, var_type):
        if var_type == "string":
            return parameter.StringParameter(var_name, asearch.child_by_name(a_soil_list, var_name).getValue())
        elif var_type == "double":
            return parameter.DoubleParameter(var_name, asearch.child_by_name(a_soil_list, var_name).getValue())
        else:
            pass

    model_par_list = asearch.find_path(xml, ["state", "model parameters"], no_skip=True)
    frz_relp_par_list = model_par_list.sublist("freezing rel perm parameters")
    try:
        soil_types = asearch.children_by_tag(asearch.remove_element(xml, 
                ["PKs", "subsurface flow", "water retention evaluator", "WRM parameters"], 
                False, True), 'ParameterList')
    except aerrors.MissingXMLError:
        soil_types = asearch.children_by_tag(asearch.child_by_name(model_par_list, "WRM parameters"), 
                                             'ParameterList')
    for s in soil_types:
        slist = frz_relp_par_list.sublist(s.getName())
        for var_name, var_type in zip(["region", "residual saturation [-]", 
            "smoothing interval width [saturation]"], ["string", "double", "double"]):
            slist.append(copy_element(s, var_name, var_type))

        slist.append(parameter.StringParameter("WRM Type", "Brooks-Corey"))
        alpha = asearch.child_by_name(s, "van Genuchten alpha [Pa^-1]").getValue()
        try:
            n = asearch.child_by_name(s, "van Genuchten n [-]").getValue()
        except:
            m = asearch.child_by_name(s, "van Genuchten m [-]").getValue()
            n = 1 / (1 - m)
        bc_satp, bc_lambda_recip = get_bc_param_from_vg(alpha, n)
        slist.append(parameter.DoubleParameter("Brooks Corey saturated matric suction [Pa]", bc_satp))
        slist.append(parameter.DoubleParameter("Brooks Corey lambda [-]", 1 / bc_lambda_recip))

                
def update(xml, rs_option="sakagucki-zeng soil resistance", 
           dessicated_zone_set=0.1, add_frz_relp=False):
    """generic update calls all needed things""" 
    add_soil_resistance(xml, rs_option)
    add_state_model_parameters(xml)
    add_wrm_to_model_parameters(xml)
    add_dessicated_zone_to_WRM(xml, dessicated_zone_set)
    del_lc_params(xml)
    if not add_frz_relp:
        add_rel_perm(xml, "WRM rel perm")
    else:
        add_rel_perm(xml, "Brooks-Corey based high frozen rel perm")
        add_frz_relp_to_model_parameters(xml)



if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("infile", help="input filename")
    parser.add_argument("--rsoil_model", default="sakagucki-zeng",
                        choices=["sakagucki-zeng", "sellers"],
                        help="soil resistance models")
    parser.add_argument("--dessicated_zone_set", default=0.1, nargs='+',
                        help="A set of dessicated zone thickness, can be a list same size and"+ 
                        "order with soil types; or a single value universal to all soil types")
    parser.add_argument("--arctic", action="store_true")

    group = parser.add_mutually_exclusive_group()
    group.add_argument("-i", "--inplace", action="store_true", help="fix file in place")
    group.add_argument("-o", "--outfile", help="output filename")

    args = parser.parse_args()


    print("Converting file: %s"%args.infile)
    xml = aio.fromFile(args.infile, True)
    if args.arctic:
        update(xml, args.rsoil_model+" soil resistance", args.dessicated_zone_set, True)
    else:
        update(xml, args.rsoil_model+" soil resistance", args.dessicated_zone_set)

    if args.inplace:
        aio.toFile(xml, args.infile)
    else:
        aio.toFile(xml, args.outfile)
    sys.exit(0)
    

    
