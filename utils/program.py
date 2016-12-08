from microbit import *
while True:
  #display.show(Image.HEART)
  #sleep(500)
  #display.show(Image.HEART_SMALL)
  #sleep(500)
  if button_a.is_pressed() and button_b.is_pressed():
    display.show(Image.ARROW_N)
  elif button_a.is_pressed():
    display.show(Image.ARROW_W)
  elif button_b.is_pressed():
    display.show(Image.ARROW_E)
  else:
    display.clear()
