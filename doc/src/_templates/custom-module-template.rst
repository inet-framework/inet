{{ name | escape | underline}}
from {{ module | escape }}

{% block modules %}
{% if modules %}
.. rubric:: Modules

.. autosummary::
   :toctree:
   :template: custom-module-template.rst
   :recursive:
{% for item in modules %}
   {%- if not item.startswith('_') %}
        {{ item }}
   {% endif %}
{%- endfor %}
{% endif %}
{% endblock %}

.. automodule:: {{ fullname }}

   {% block attributes %}
   {% if attributes %}
   .. rubric:: Module attributes

   .. currentmodule:: {{ fullname }}

   .. autosummary::
   {% for item in attributes %}
      {%- if not item.startswith('_') %}
        {{ item }}
      {% endif %}
   {%- endfor %}
   {% endif %}
   {% endblock %}

   {% block functions %}
   {% if functions %}
   .. rubric:: {{ _('Functions') }}

   .. currentmodule:: {{ fullname }}

   .. autosummary::
      :toctree:
      :nosignatures:
   {% for item in functions %}
      {%- if not item.startswith('_') %}
        {{ item }}
      {% endif %}
   {%- endfor %}
   {% endif %}
   {% endblock %}

   {% block classes %}
   {% if classes %}
   .. rubric:: {{ _('Classes') }}

   .. currentmodule:: {{ fullname }}

   .. autosummary::
      :toctree:
      :template: custom-class-template.rst
      :nosignatures:
   {% for item in classes %}
      {%- if not item.startswith('_') %}
        {{ item }}
      {% endif %}
   {%- endfor %}
   {% endif %}
   {% endblock %}

   {% block exceptions %}
   {% if exceptions %}
   .. rubric:: {{ _('Exceptions') }}

   .. currentmodule:: {{ fullname }}

   .. autosummary::
   {% for item in exceptions %}
      {%- if not item.startswith('_') %}
        {{ item }}
      {% endif %}
   {%- endfor %}
   {% endif %}
   {% endblock %}

