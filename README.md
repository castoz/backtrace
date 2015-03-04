# backtrace
A user-space simulation dump_stack(), based on mips.
It supplies two APIs: 
    show_backtrace(): show backtrace of function caller tree.
    addr_to_name(): given an addr, get the function name it belongs to.
