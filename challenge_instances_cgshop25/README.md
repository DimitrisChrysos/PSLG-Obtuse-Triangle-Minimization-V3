# Challenge Instances for CG:SHOP 2025

These are the instances on which you are benchmarked in the CG:SHOP 2025 competition.

The schema has the following fields:

1. **instance_uid**
    - **Type**: string
    - **Description**: A unique identifier for the instance.
2. **num_points**
    - **Type**: integer
    - **Description**: The total number of points in the instance. All points must be included in the final triangulation.
3. **points_x**
    - **Type**: array of integers
    - **Description**: A list containing the x-coordinates of the points.
4. **points_y**
    - **Type**: array of integers
    - **Description**: A list containing the y-coordinates of the points.
5. **region_boundary**
    - **Type**: array of integers
    - **Description**: A list of point indices representing the boundary of the region to be triangulated. The points are oriented counter-clockwise, and the first point is not repeated at the end of the list. The triangulation may split the boundary segments.
6. **num_constraints**
    - **Type**: integer
    - **Description**: The number of additional constraints in the instance.
7. **additional_constraints**
    - **Type**: array of arrays of integers
    - **Description**: A list of additional constraints, where each constraint is a pair of point indices. The triangulation may split these constraint segments but must include a straight line between the two points.

An instance may look like follows:

```json
{
 "instance_uid": "simple_polygon_with_exterior_20_080",
 "num_points": 20,
 "points_x": [632,1330,3051,5040,5883,8130,9280,9613,9422,8996,8020,8467,6735,4674,2519,973,1205,1929,3203,5345],
 "points_y": [1588,1097,470,1077,2766,3629,2836,4963,6363,7327,7611,9720,9183,7865,7692,9797,6005,5812,6301,2923],
 "region_boundary": [0,1,2,3,6,7,8,11,15],
 "num_constraints": 0,
 "additional_constraints": [
  [3,4],
  [5,6],
  [9,10],
  [10,11],
  [11,12],
  [12,13],
  [13,14],
  [14,15],
  [15,16],
  [18,19],
  [19,0]
 ]
}
```

Visualizations for the instances have been provided to compare to your parser.
Note that the competition comes with Python utils [pyutils25](https://github.com/CG-SHOP/pyutils25) to read and verify instances and solutions.

## Changes

- 2024-09-30: Initial version.
- 2024-10-08: The `num_constraints` field was not properly populated in the instances. This has been fixed.