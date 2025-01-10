- Περιγραφή του προγράμματος:
    - Πιστεύουμε πως το πρόγραμμα μας υλοποιεί όλες τις απαιτήσεις της εργασίας που μας ζητήθηκαν μέσω της εκφώνησης, των φροντιστηρίων και των συζητήσεων στο eclass.
    - Το ex.cpp είναι το αρχείο με την main που καλεί τις απαραίτητες συναρτήσεις για την υλοποίηση της εργασίας μας.
    - Αρχικά διαβάζουμε κάποιο json αρχείο ως input.
    - Κατασκευάζουμε το cdt με τις οδηγίες που μας δίνονται.
    - Διαλέγουμε τη μέθοδο (ή τις μεθόδους) που θα χρησιμοποιηθούν και τις παραμέτρους τους, από το αρχείο που διαβάστηκε.
    - Βάζουμε τα απαραίτητα steiner points για να μηδενίσουμε όσο γίνεται τα αμβλυγώνια τρίγωνα.
    - Δημιουργούμε το output αρχείο json.



- Κατάλογος των αρχείων κώδικα και το ρόλο τους:
1) Αρχείο ex.cpp στο βασικό directory:
    1) Εκτελεί την παραπάνω συνοπτική περιγραφή που αναφέρθηκε.
    2) Χρησιμοποιεί της συναρτήσεις:
        - handle_methods(), ανάλογα τις παραμέτρους επιλέγει τη μέθοδο και το τροόπο που θα χρησιμοποιηθεί:
            1) local_search()
            2) sim_annealing()
            3) ant_colony_optimization()
            - Κάθε μέθοδος χρησιμοποιεί βοηθητικές συναρτήσεις και δομές
        - write_output(), δημιουργεί το output json αρχείο
2) Κατάλογος includes:
    1) Υποκατάλογος custom_cdt_class:
        1. Αρχείο custom_cdt_class.hpp, περιέχει την custom κλάση που δημιουργήθηκε για τις ανάγκες των μεθόδων:
            - insert_no_flip()
            - insert_steiner_x_y(), αποθηκεύει σε ένα vertex στο cdt τις συντεταγμένες των steiner points που εισάγουμε
            - my_is_flippable(), συμβάλει στον έλεγχο για το αν μία ακμή έχει τη δυνατότητα να γίνει flipped
            - remove_no_flip()
            - Ορίζει κάποια typedef για τις ανάγκες όλων των αρχείων της εργασίας
        2. Αρχείο custom_cdt_class.cpp, βοηθητικό για την μεταγλώττιση
    2) Υποκατάλογος read_write_file:
        1. Αρχείο read_write_file.hpp, περιέχει τη δήλωση όλων των βοηθητικών συναρτήσεων για την ανάγνωση του αρχείου json και για τη δημιουργία του output αρχείο json
        2. Αρχείο read_write_file.cpp, περιέχει την υλοποίηση όλων των βοηθητικών συναρτήσεων για την ανάγνωση του αρχείου json και για τη δημιουργία του output αρχείο json
    3) Υποκατάλογος steiner_methods:
        1. Αρχείο steiner_methods.hpp περιέχει τη δήλωση των 5 μεθόδων steiner και μίας βοηθητικής
        2. Αρχείο steiner_methods.cpp περιέχει την υλοποίηση των 5 μεθόδων steiner και μίας βοηθητικής
    4) Υποκατάλογος utils:
        1. Αρχείο utils.hpp περιέχει τη δήλωση των περισσότερων βοηθητικών μεθόδων και κλάσεων
        2. Αρχείο utils.cpp περιέχει την υλοποίηση των περισσότερων βοηθητικών μεθόδων και κλάσεων
3) Κατάλογος outputs:
    - Περιέχει και για τις τρεις μεθόδους (ls, sa, ant) τα outputs των test που μας δόθηκαν
4) Κατάλογοι test:
    - Περιέχουν τα test που μας δόθηκαν για τις τρεις μεθόδους



- Οδηγίες μεταγλώττισης:
    1) cmake -DCGAL_DIR=/usr/lib/CGAL
    2) make



- Οδηγίες χρήσης του προγράμματος:
    1) Για το example τρέχουμε:
        - ls: ./opt_triangulation -i input_ls.json -o outputs/output_ls.json
        - sa: ./opt_triangulation -i input_sa.json -o outputs/output_sa.json
        - ant: ./opt_triangulation -i input_ant.json -o outputs/output_ant.json
    2) Για τα tests:
        - ls: ./opt_triangulation -i tests_ls/instance_1.json -o outputs/tests_ls/output1.json
        - sa: ./opt_triangulation -i tests_sa/instance_1.json -o outputs/tests_sa/output1.json
        - ant: ./opt_triangulation -i tests_ant/instance_1.json -o outputs/tests_ant/output1.json
        - Αντίστοιχα αλλάζουμε του αριθμούς 1, για όποιο test θέλουμε
    3) Για τα test_instances:
        - ls: ./opt_triangulation -i test_instances_ls/instance_test_1.json -o outputs/test_instances_ls/output1.json
        - sa: ./opt_triangulation -i test_instances_sa/instance_test_1.json -o outputs/test_instances_sa/output1.json
        - ant: ./opt_triangulation -i test_instances_ant/instance_test_1.json -o outputs/test_instances_ant/output1.json
        - Αντίστοιχα αλλάζουμε του αριθμούς 1, για όποιο test θέλουμε



- Πλήρη στοιχεία των φοιτητών που ανέπτυξαν την εργασία:
1) Ονοματεπώνυμο: Αναστάσιος Μουμουλίδης - ΑΜ: 1115202100108
2) Ονοματεπώνυμο: Δημήτριος Χρυσός - ΑΜ: 1115202100275
    


- Επιλογή παραμέτρων:
    1) Η επιλογή των παραμέτρων, έγινε για κάθε input json αρχείο και μέθοδο ξεχωριστά
    2) Η επιλογή έγινε με βάση δικές μας δοκιμές και τα αποτελέσματα που βγάζαμε



- Σύγκριση μεθόδων:
    1) Με βάση τα αρχεία output που αναφερθήκαμε προηγουμένως, είναι φανερό ότι η πιο αποτελεσματική μέθοδος είναι η local search, μετά η simulated annealing και μετά η ant colony optimization
    2) Επίσης παρατηρείται ότι η χρήση της παραμέτρου, "delaunay": false, βοηθάει μερικές φορές κυρίως τις μεθόδους sa και ants αλλά χωρίς σταθερά αποτελέσματα για κάθε input



- Test if the input categorization works:
    1. Convex no constr: 
    ./opt_triangulation -i challenge_instances_cgshop25/point-set_40_ae33a7ea.instance.json -o outputs/output1.json -preselected_params

    2. Convex open constr: 
    ./opt_triangulation -i challenge_instances_cgshop25/simple-polygon-exterior-20_10_ce9152de.instance.json -o outputs/output1.json -preselected_params

    3. Convex closed constr: 
    ./opt_triangulation -i challenge_instances_cgshop25/simple-polygon-exterior_10_310dc6c7.instance.json -o outputs/output1.json -preselected_params

    4. Not convex parallel: 
    ./opt_triangulation -i challenge_instances_cgshop25/ortho_10_d2723dcc.instance.json -o outputs/output1.json -preselected_params

    5. Not convex no rules: 
    ./opt_triangulation -i challenge_instances_cgshop25/simple-polygon_10_272aa6ea.instance.json -o outputs/output1.json -preselected_params


- Arguments:
    1. argument 5:
        1) argv[5] = "-preselected_params"
        2) argv[5] = "-ls" (or) "-sa" (or) "-ant" // to choose the method
    2. argument 6 and more are used to choose which steiner methods are available:
        1) argv[6+] = "-proj" 
        2) argv[6+] = "-centr" 
        3) argv[6+] = "-mid" 
        4) argv[6+] = "-circum" 
        5) argv[6+] = "-merge" 

- Test that things work (for the choose which method and steiner method to use feature), by using those:
    1. ./opt_triangulation -i old_tests_and_outputs/input_ls.json -o outputs/output1.json
    2. ./opt_triangulation -i challenge_instances_cgshop25/simple-polygon_10_272aa6ea.instance.json -o outputs/output1.json -preselected_params
    3. ./opt_triangulation -i challenge_instances_cgshop25/simple-polygon_10_272aa6ea.instance.json -o outputs/output1.json -ls -proj -mid -centr -circum -merge
    4. ./opt_triangulation -i challenge_instances_cgshop25/simple-polygon_10_272aa6ea.instance.json -o outputs/output1.json -ls -proj
    5. ./opt_triangulation -i challenge_instances_cgshop25/simple-polygon_10_272aa6ea.instance.json -o outputs/output1.json -ls -mid
    6. ./opt_triangulation -i old_tests_and_outputs/input_sa.json -o outputs/output1.json
    7.  ./opt_triangulation -i challenge_instances_cgshop25/simple-polygon_10_272aa6ea.instance.json -o outputs/output1.json -sa -proj
    8. ./opt_triangulation -i challenge_instances_cgshop25/simple-polygon_10_272aa6ea.instance.json -o outputs/output1.json -sa -proj -mid
    9. ./opt_triangulation -i old_tests_and_outputs/input_ant.json -o outputs/output1.json
    10. ./opt_triangulation -i challenge_instances_cgshop25/simple-polygon_10_272aa6ea.instance.json -o outputs/output1.json -ant -proj
    11. ./opt_triangulation -i challenge_instances_cgshop25/simple-polygon_10_272aa6ea.instance.json -o outputs/output1.json -ant -proj -mid -centr -circum -merge

- alpha = 3
- beta = 0.3 

- Testing instances with their subsets of methods and steiner methods:
    1) Category A -> Convex Boundary - No Constraints
        - point-set_10_4bcb7c21.instance.json
        - point-set_10_7451a2a9.instance.json
        - point-set_20_fa3fd7e0.instance.json
        - point-set_40_8cbf31aa.instance.json
        - point-set_40_9451c229.instance.json
        - point-set_60_27bc003d.instance.json
        - point-set_80_ff15444b.instance.json
        - point-set_80_d77fb670.instance.json
        - point-set_100_dd67678e.instance.json
        - point-set_150_982c9ab3.instance.json
    2) Category B -> Convex Boundary - Open Constrants
        - simple-polygon-exterior-20_10_6fbd9669.instance.json
        - simple-polygon-exterior-20_10_ce9152de.instance.json
        - simple-polygon-exterior-20_20_4ddfa00e.instance.json
        - simple-polygon-exterior-20_60_57858065.instance.json
        - simple-polygon-exterior-20_100_512f0fc4.instance.json
        - simple-polygon-exterior-20_100_8d1c2e30.instance.json
        - simple-polygon-exterior-20_100_8ff7a64d.instance.json
    3) Category C -> Convex Boundary - Close Constraints
        - simple-polygon-exterior_10_c5616894.instance.json
        - simple-polygon-exterior-20_10_8c4306da.instance.json
        - simple-polygon-exterior-20_60_28a85662.instance.json
        - simple-polygon-exterior_40_785575e7.instance.json
        - simple-polygon-exterior_40_ff947945.instance.json
        - simple-polygon-exterior_60_ba2c82c0.instance.json
        - simple-polygon-exterior_80_22d34c7e.instance.json
        - simple-polygon-exterior_100_f1740925.instance.json
        - simple-polygon-exterior_150_1301b82e.instance.json
        - simple-polygon-exterior_250_a97729dd.instance.json
    4) Category D -> Not Convex Boundary - Parallel to Axes
        - ortho_10_d2723dcc.instance.json
        - ortho_20_5a9e8244.instance.json
        - ortho_20_e2aff192.instance.json
        - ortho_40_56a6f463.instance.json
        - ortho_60_5c5796a0.instance.json
        - ortho_60_c423f527.instance.json
        - ortho_80_06ee55d4.instance.json
        - ortho_100_bd1e4a14.instance.json
        - ortho_150_53eb4022.instance.json
        - ortho_250_3b977f7e.instance.json
    5) Category E -> Not Convex Boundary - No Rules
        - simple-polygon_10_272aa6ea.instance.json
        - simple-polygon_20_0dda68ed.instance.json
        - simple-polygon_40_12969fc3.instance.json
        - simple-polygon_60_17af118a.instance.json
        - simple-polygon_80_7b8f6c4c.instance.json
        - simple-polygon_100_6101abad.instance.json
        - simple-polygon_100_cb23308c.instance.json
        - simple-polygon_150_743d6b9c.instance.json
        - simple-polygon_250_432b4814.instance.json
        - simple-polygon_250_6e9d9c26.instance.json
    

- github repo link: https://github.com/DimitrisChrysos/project-emiris-3.git