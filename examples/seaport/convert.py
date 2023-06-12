import sys

def create_xml_from_coordinates(coordinate_string):
    coordinates = list(map(float, coordinate_string.split()))
    xml_string = ""
    for i in range(0, len(coordinates), 2):
        x = coordinates[i]
        y = coordinates[i+1]
        xml_string += f'<moveto x="{x}" y="{y}"/>\n'
    return xml_string

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Please provide one string of space-separated coordinates as an argument.")
    else:
        coordinate_string = sys.argv[1]
        print(create_xml_from_coordinates(coordinate_string))
