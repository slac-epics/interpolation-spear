# $File: //ASP/Dev/SBS/4_Controls/4_4_Equipment_Control/4_4_8_Insertion_Device_Control/sw/Feed_Forward/Feed_Forward_CommonApp/Db/Makefile $
# $Revision: 1.1.1.1 $
# $DateTime: 2012/06/07 10:30:50 $
# Last checked in by: $Author: saa $

TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#----------------------------------------------------
#  Optimization of db files using dbst (DEFAULT: NO)
#DB_OPT = YES

#----------------------------------------------------
# Create and install (or just install) into <top>/db
# databases, templates, substitutions like this
#
DB += Feed_Forward_1D_Input_Validation.template
DB += Feed_Forward_2D_Input_Validation.template
DB += Feed_Forward_3D_Input_Validation.template
DB += Quadrupole_Interpolation_Control.template
DB += Quadrupole_Interpolation_Calculations.template
DB += Quadrupole_Interpolation_Tables.template
DB += Quadrupole_Zero_Trim_Current.template

#
#
CRAPI += feed_forward_sector_autosave_template.req
CRAPI += feed_forward_source_target_autosave_template.req


#----------------------------------------------------
# If <anyname>.db template is not named <anyname>*.template add
# <anyname>_template = <templatename>

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

# end