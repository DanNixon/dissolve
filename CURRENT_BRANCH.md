# Current Branch Worklist

## Main
- Add OnlyWhenEnergyStable option to MDModule
- Add System Templates to startup screen, which provide skeletal system setups for common (or not-so-common) use cases, but without any details of processing (which is the job of the Add Layer Wizard - contents of that could be called from the Templates wizard). For example: pure liquid, liquid mixture, A solvated in water, A+B solvated in water, silica, A confined in MCM-41, water, A+B confined in MCM-41.  Other examples, such as "A absorbed in MOF", need to ask for a cif file for the MOF, for instance. Store as individual, complete input files somewhere, which are somehow stored (in Qt resource, if we only care about accessing them via the GUI?). Can test these files, loading them as input to dissolve-serial, to check for consistency.
- Make Module::frequency_ default to 1 in all cases?  Then suggests layers for shake / calc / refine, which is quite neat... Still allow adjusting.

## Follow On
- [AS ISSUE] Control Layer: A layer that manages the execution of processing layers, enabling regeneration of configurations, minimisation, refinement, testing, fitting of parameters. With other proposed changes to allow procedure definitions for configuration creation, this could be the best route to allow searching through this type of parameter space. Can dump a restart file at each 'good' or 'best' point, rather than every 'n' iterations of the main loop. Control Layer could be present all the time, but the default option is just to continually iterate over the layers.