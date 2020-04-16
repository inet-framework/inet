def solid_x_axis(ax, default_color):
    a = ax.get_ygridlines()
    ticklocs = ax.get_yticks()
    ticklabels = ax.get_yticklabels()
    #print("gridlines:",a)
    
    j = 0
    r = 0
    # set everything to default here
    ax.grid(linestyle='--',color=default_color)
    
    for i in ticklocs:
        if i == 0.0:
            #print("found 0 at ",j)
            r = j
            b = a[r]
            b.set_color('black')
            b.set_linestyle('solid')
        j += 1