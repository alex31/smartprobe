# STMEMS  file

STMEMSSRCDIR = $(EXTLIB)/STMems_Standard_C_drivers/
STMEMSLPS33HWDIR = $(STMEMSSRCDIR)/lps33hw_STdC/driver
STMEMSLPS33HWSRC = $(STMEMSLPS33HWDIR)/lps33hw_reg.c


# Shared variables
ALLSRC += $(STMEMSLPS33HWSRC)
ALLINC  += $(STMEMSLPS33HWDIR)


