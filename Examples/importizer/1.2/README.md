# Handle non-local macros, transitive includes, and forward declaration
- There is no non-local macros in sight, my project use strict IWYU, but there are some forward declarations that I need to handle. 
- Luckily, I don't have any cyclic dependency, so I am OK.
