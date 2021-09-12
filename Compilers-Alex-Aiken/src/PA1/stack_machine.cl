(* Stack implementation *)
class StackEntry {
    command: StackCommand;
    next:    StackEntry;

    command(): StackCommand { command };

    next(): StackEntry { next };

    init(entry: StackCommand, head: StackEntry): StackEntry {
        {
            command <- entry;
            next <- head;
            self;
        }
    };
};

class Stack {
    head: StackEntry;

    push(entry: StackCommand) : StackCommand {
        (head <- (new StackEntry).init(entry, head)).command()
    };

    pop(): StackCommand {
        if (isvoid head) then {
            (new IO).out_string("Stack is empty or operation needs more operands!\n");
            abort();
            new StackCommand;   (* or compilation error *)
        } else
        let command: StackCommand <- head.command() in {
            head <- head.next();
            command;
        }
        fi
    };

    display(): Object {
        let current: StackEntry <- head,
            io: IO <- new IO in {
            while (not isvoid current) loop {
                io.out_string(current.command().display());
                io.out_string("\n");
                current <- current.next();
            }
            pool;
        }
    };

    evaluate(): StackCommand {
        if (not isvoid head) then
            let command: StackCommand <- head.command() in
                command.evaluate(self)
        else
            head.command()
        fi
    };
};

(* Commands implementation *)
class StackCommand {
    evaluate(s: Stack): StackCommand {
        {
            (new IO).out_string("Don't call StackCommand::evaluate()!");
            abort();
            new StackCommand; (* or compilation error *)
        }
    };

    display(): String {
        {
            (new IO).out_string("Don't call StackCommand::display()!");
            abort();
            "";  (* or compilation error *)
        }
    };
};

class Integer inherits StackCommand {
    value: Int;

    init(v: Int) : Integer {
        {
            value <- v;
            self;
        }
    };

    value(): Int { value };

    evaluate(s: Stack): StackCommand { self };

    display(): String {
        (new A2I).i2a(value)
    };
};

class Plus inherits StackCommand {
    evaluate(s: Stack): StackCommand {
        {
            s.pop();
            let a: StackCommand <- s.pop(),
                b: StackCommand <- s.pop() in {
                    let a_i: Int <-
                        case a of
                            i: Integer => i.value();
                            o: Object => {
                                (new IO).out_string("Not Integer!\n");
                                abort(); 0;
                            };
                        esac,
                    b_i: Int <-
                        case b of
                            i: Integer => i.value();
                            o: Object => {
                                (new IO).out_string("Not Integer!\n");
                                abort();
                                0;
                            };
                        esac
                    in
                        s.push((new Integer).init(a_i + b_i));
            };
        }
    };

    display(): String {
        "+"
    };
};

class Swap inherits StackCommand {
    evaluate(s: Stack): StackCommand {
        {
            s.pop();
            let a: StackCommand <- s.pop(),
                b: StackCommand <- s.pop() in {
                    s.push(a);
                    s.push(b);
                };
        }
    };

    display(): String {
        "s"
    };
};

(* No bad input checking! *)
class Main inherits IO {
    main(): Object {
        {
            out_string(">");
            let input: String,
                stack: Stack <- new Stack,
                io: IO <- new IO,
                converter: A2I <- new A2I in
            while (not (input <- io.in_string()) = "x") loop {
                if (input = "+") then
                    stack.push(new Plus)
                else if (input = "s") then
                    stack.push(new Swap)
                else if (input = "d") then
                    stack.display()
                else if (input = "e") then
                    stack.evaluate()
                else
                    stack.push((new Integer).init(converter.a2i(input)))
                fi fi fi fi;
                out_string(">");
            }
            pool;
        }
    };
};