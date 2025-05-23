from cgshop2025_pyutils.data_schemas.instance import Cgshop2025Instance

from cgshop2025_pyutils.data_schemas.solution import Cgshop2025Solution

from cgshop2025_pyutils import verify

import json

# import argparse



# parser = argparse.ArgumentParser(description="Verify the output of the triangulation program.")
# parser.add_argument("input_filepath", type=str, help="Path to the input JSON file.")
# parser.add_argument("output_filepath", type=str, help="Path to the output JSON file.")
# args = parser.parse_args()
# input_filepath = args.input_filepath
# output_filepath = args.output_filepath

input_filepath = "challenge_instances_cgshop25/simple-polygon_10_272aa6ea.instance.json"
# input_filepath = "test_instances_ls/instance_test_4.json"
# input_filepath = "challenge_instances_cgshop25/point-set_40_ae33a7ea.instance.json"

output_filepath = "outputs/test_instances/simple-polygon_10_272aa6ea.json"
# output_filepath = "outputs/test_instances_ls/output4.json"
# output_filepath = "outputs/output1.json"


with open(input_filepath, 'r') as file:

  data_in = json.load(file)



with open(output_filepath, 'r') as file:

  data_out = json.load(file)




instance = Cgshop2025Instance(

            instance_uid=data_in["instance_uid"],

            num_points=data_in["num_points"],

            points_x=data_in["points_x"],

            points_y=data_in["points_y"],

            region_boundary=data_in["region_boundary"],

            num_constraints=data_in["num_constraints"],

            additional_constraints=data_in["additional_constraints"],

          )



solution = Cgshop2025Solution (

              content_type="CG_SHOP_2025_Solution",

              instance_uid=data_out["instance_uid"],

              steiner_points_x=data_out["steiner_points_x"],

              steiner_points_y=data_out["steiner_points_y"],

              edges=data_out["edges"],

            )



result = verify(instance, solution, strict=True)



if result.num_obtuse_triangles != -1:

  print(f"No. obtuse triangles: {result.num_obtuse_triangles}\nNo. Steiner points: {result.num_steiner_points}")

else:

  print("Errors:")

  for err in result.errors: print(f"- {err}")  