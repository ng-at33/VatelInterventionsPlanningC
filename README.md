# VatelInterventionsPlanningC

![](logo-vatel-bordeaux.jpeg)

Small decisions support tool to help schedule interventions between groups of students and professionals in the hospitality sector.

This work deals with the planning of professional interventions within a management school. There is a marketing and management school operating in the hospitality sector located in the area of Bordeaux. The school has its students usually working for different establishments (bars, restaurants and hotels) for 14 days every 28 days. During the Covid pandemic, all of theses establishments being closed, both students and professionals employed in those establishments can work together in a different way : the school schedules interventions lead by the professionals to different groups of students. The goal of the system described in this document is to help scheduling those interventions based on different criteria.

More doc on the algorithm used coming soon...

## Dependent libraries
- OpenXLSX

## TODO (more or less in priority order):
- add colors in solution display
- improve performance by not generating every mutation of a node but instead heuristically choose more strategic ones (try to remove assignations from crowded slots for example). Actually the algorithm gives good results on real data which is more important, but for data instances where professionals are available on more slots it can be very slow (>5 minutes instead of <1 second)
- read parameters used in main in file/command line rather than hard coding them
- use different searching functions (findAsGr, findAsPr) to improve performance
- improve solution quality: add new assignations
- improve performance: start with a better first node by sorting groups ?
- implement other type of mutations: change a professional in a node for example