figure()

subplot(fila,columna,numero)
plot,stem,stair,hist
title('')
axis([xmin xmax y min ymax])

%% Si se quiero poner un titulo general a la grafica pongo:
sgtitle('')

y = mu + sigma*randn(N,1)
[h, xbins] = hist(y,bins(nº de barras))
bin_width = xbins(2)-xbins(1)
h_norm = h/N/bin_width

bar(xbins,h_norm,1)

