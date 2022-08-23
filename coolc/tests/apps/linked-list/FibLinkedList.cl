class FibNode inherits IO {
    value: Int;
    next : FibNode;

    initFibNode(val: Int) : FibNode {
        {
            value <- val;
            self;
        }
    };

    value() : Int {
        value
    };

    next() : FibNode {
        next
    };

    newFibNode(lhs: FibNode, rhs: FibNode) : FibNode {
        next <- (new FibNode).initFibNode(lhs.value() + rhs.value())
    };

    newNextFibNode(val: Int) : FibNode {
        next <- (new FibNode).initFibNode(val)
    };

    print() : FibNode {
        out_int(value).out_string("\n")
    };
};

class Main {
    root : FibNode <- (new FibNode).initFibNode(0);
    
    curr : FibNode;
    prev : FibNode;

    x: Int;

    main() : Object {
        {
            prev <- root;
            curr <- root.newNextFibNode(1);

            while x < 10000000 loop {
                if (x - (x / 100) * 100 = 0)
                then
                    root <- root.next()
                else
                    root
                fi;

                let tmp: FibNode <- prev in {
                    prev <- curr;
                    curr <- curr.newFibNode(tmp, curr);
                };

                x <- x + 1;
            } pool;
        }
    };
};