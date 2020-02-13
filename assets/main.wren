System.print("Test from inside the VM in main.wren")

foreign class Input {
    construct new(){}
    foreign is_key_pressed()
}

var input = Input.new()

class Game {
  construct new(input){
    _input = input
  }
  update(elapsedTime) {
    if (_input.is_key_pressed()) {
        System.print("key pressed")
    }
    //System.print("update")
  }
}

var game = Game.new(input)
