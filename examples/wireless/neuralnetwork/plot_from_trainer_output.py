
outp = """Predicted: 0.04500734433531761, actual: 0.0
Predicted: 0.09648394584655762, actual: 0.04
Predicted: 0.3102042078971863, actual: 0.22
Predicted: 0.9753725528717041, actual: 1.0
Predicted: 0.04369208961725235, actual: 0.01
Predicted: 0.4756568968296051, actual: 0.28
Predicted: 0.9756531715393066, actual: 1.0
Predicted: 0.12722794711589813, actual: 0.08
Predicted: 0.9771522283554077, actual: 1.0
Predicted: 0.977291464805603, actual: 1.0
Predicted: 0.9732115268707275, actual: 0.86
Predicted: 0.9539188146591187, actual: 1.0
Predicted: 0.6446861624717712, actual: 0.33
Predicted: 0.9181270599365234, actual: 0.91
Predicted: 0.9752393960952759, actual: 1.0
Predicted: 0.9772660136222839, actual: 1.0
Predicted: 0.04674476757645607, actual: 0.02
Predicted: 0.97666996717453, actual: 1.0
Predicted: 0.10348676145076752, actual: 0.01
Predicted: 0.7879534959793091, actual: 0.99
Predicted: 0.9763076901435852, actual: 1.0
Predicted: 0.9687694907188416, actual: 0.99
Predicted: 0.04454422369599342, actual: 0.05
Predicted: 0.6632455587387085, actual: 0.28
Predicted: 0.9764233231544495, actual: 1.0
Predicted: 0.18601398169994354, actual: 0.02
Predicted: 0.9771592617034912, actual: 1.0
Predicted: 0.5626338124275208, actual: 0.49
Predicted: 0.7365133166313171, actual: 0.57
Predicted: 0.04387374967336655, actual: 0.0
Predicted: 0.8451017141342163, actual: 0.97
Predicted: 0.9764474630355835, actual: 1.0
Predicted: 0.9770940542221069, actual: 1.0
Predicted: 0.9748854041099548, actual: 0.9
Predicted: 0.8188756704330444, actual: 0.89
Predicted: 0.045427899807691574, actual: 0.0
Predicted: 0.9738456606864929, actual: 1.0
Predicted: 0.9769092798233032, actual: 1.0
Predicted: 0.9733941555023193, actual: 1.0
Predicted: 0.052596986293792725, actual: 0.11
Predicted: 0.9718091487884521, actual: 1.0
Predicted: 0.05463666468858719, actual: 0.0
Predicted: 0.6977971196174622, actual: 0.66
Predicted: 0.9738655090332031, actual: 0.96
Predicted: 0.7937473654747009, actual: 0.69
Predicted: 0.976258397102356, actual: 1.0
Predicted: 0.9713939428329468, actual: 0.98
Predicted: 0.9480686783790588, actual: 0.98
Predicted: 0.4418124258518219, actual: 0.46
Predicted: 0.9768193364143372, actual: 1.0
Predicted: 0.09463569521903992, actual: 0.3
Predicted: 0.9768067002296448, actual: 1.0
Predicted: 0.6994383931159973, actual: 0.51
Predicted: 0.593998908996582, actual: 0.88
Predicted: 0.9770746231079102, actual: 1.0
Predicted: 0.966774582862854, actual: 0.95
Predicted: 0.8056410551071167, actual: 0.8
Predicted: 0.9472115635871887, actual: 1.0
Predicted: 0.9771569967269897, actual: 1.0
Predicted: 0.9661707878112793, actual: 0.68
Predicted: 0.9771852493286133, actual: 1.0
Predicted: 0.4251471161842346, actual: 0.22
Predicted: 0.039817195385694504, actual: 0.0
Predicted: 0.9738278388977051, actual: 0.95
Predicted: 0.04301508516073227, actual: 0.03
Predicted: 0.9770622253417969, actual: 1.0
Predicted: 0.29970449209213257, actual: 0.2"""

lines = outp.split("\n")

nums = [l.split(',') for l in lines]

nums = [ (float(n[0].split(':')[1].strip()), float(n[1].split(':')[1].strip())) for n in nums]
import matplotlib.pyplot as plt

a, b = zip(*nums)
plt.scatter(a, b)
plt.show()
