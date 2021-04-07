# VatelInterventionsPlanningC

Small decisions support tool to help schedule interventions between groups of students and professionals in the hospitality sector.

This work deals with the planning of professional interventions within a management school. There is a marketing and management school operating in the hospitality sector located in the area of Bordeaux. The school has its students usually working for different establishments (bars, restaurants and hotels) for 14 days every 28 days. During the Covid pandemic, all of theses establishments being closed, both students and professionals employed in those establishments can work together in a different way : the school schedules interventions lead by the professionals to different groups of students. The goal of the system described in this document is to help scheduling those interventions based on different criteria.

More doc on the algorithm used coming soon...

TODO:
- implement other type of mutations: change a professional or students group in a node. This is the next prority since there are students groups that have no assignations
- improve code readability by separating node creation in functions
- improve performance by not generating every mutation of a node but instead heuristically choose more strategic ones (try to remove assignations from crowded slots for example)
- put constants in a separate declaration file